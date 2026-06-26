"""
Serial transport layer for Pclika bridge.

Manages USB-Serial connection to the device, auto-discovery,
handshake, and per-command request/response matching.
"""

from __future__ import annotations

import logging
import threading
import time
from typing import Callable

import serial
import serial.tools.list_ports

from .exceptions import DeviceNotConnected, DeviceTimeout, DeviceError, ProtocolError
from .protocol import make_command, DeviceResponse

logger = logging.getLogger("pclika.transport")

DEFAULT_BAUD      = 115200
HANDSHAKE_TIMEOUT = 3.0    # seconds
COMMAND_TIMEOUT   = 5.0    # seconds
RECONNECT_DELAY   = 2.0    # seconds


class SerialTransport:
    """Thread-safe serial transport with request-response matching."""

    def __init__(self, port: str | None = None, baud: int = DEFAULT_BAUD):
        self._port_hint  = port
        self._baud       = baud
        self._serial: serial.Serial | None = None
        self._lock       = threading.Lock()
        self._pending: dict[str, threading.Event] = {}
        self._responses: dict[str, DeviceResponse] = {}
        self._log_buffer: list[str] = []
        self._reader_thread: threading.Thread | None = None
        self._running = False
        self._on_disconnect: Callable | None = None

    # ── Connection ────────────────────────────────────────────────────────────

    def connect(self) -> str:
        """Connect to device. Returns port name."""
        port = self._port_hint or self._discover_port()
        logger.info("Connecting to %s at %d baud…", port, self._baud)

        ser = serial.Serial(port, self._baud, timeout=0.1)
        time.sleep(0.5)      # let device reset if needed
        ser.reset_input_buffer()

        self._serial  = ser
        self._running = True
        self._reader_thread = threading.Thread(
            target=self._reader_loop, daemon=True, name="pclika-reader"
        )
        self._reader_thread.start()

        self._handshake()
        logger.info("Connected and handshake OK on %s", port)
        return port

    def disconnect(self):
        """Gracefully disconnect."""
        self._running = False
        if self._serial and self._serial.is_open:
            self._serial.close()
        self._serial = None

    @property
    def connected(self) -> bool:
        return self._serial is not None and self._serial.is_open

    # ── Command execution ─────────────────────────────────────────────────────

    def command(self, cmd: str, params: dict | None = None,
                timeout: float = COMMAND_TIMEOUT) -> dict:
        """Send a command and wait for the matching response. Returns data dict."""
        if not self.connected:
            raise DeviceNotConnected("No device connected")

        frame_id, line = make_command(cmd, params)
        event = threading.Event()

        with self._lock:
            self._pending[frame_id] = event

        try:
            self._serial.write(line.encode())
            self._serial.flush()
            logger.debug("→ %s", line.rstrip())
        except serial.SerialException as exc:
            with self._lock:
                self._pending.pop(frame_id, None)
            raise DeviceNotConnected(str(exc)) from exc

        if not event.wait(timeout):
            with self._lock:
                self._pending.pop(frame_id, None)
                self._responses.pop(frame_id, None)
            raise DeviceTimeout(f"Command '{cmd}' timed out after {timeout}s")

        with self._lock:
            resp = self._responses.pop(frame_id)

        if not resp.ok:
            raise DeviceError(resp.error_code, resp.error_msg)

        return resp.data

    # ── Log buffer access ─────────────────────────────────────────────────────

    def get_log_lines(self, n: int = 50) -> list[str]:
        """Return the last n log lines captured from the device."""
        with self._lock:
            return list(self._log_buffer[-n:])

    # ── Internal ──────────────────────────────────────────────────────────────

    def _reader_loop(self):
        """Background thread: read lines from serial, dispatch responses."""
        while self._running:
            try:
                if not self._serial or not self._serial.is_open:
                    break
                raw = self._serial.readline()
                if not raw:
                    continue
                line = raw.decode(errors="replace").strip()
                if not line:
                    continue
                logger.debug("← %s", line)

                if line.startswith("{"):
                    self._dispatch_response(line)
                else:
                    # Plain text log line from device
                    with self._lock:
                        self._log_buffer.append(line)
                        if len(self._log_buffer) > 500:
                            self._log_buffer = self._log_buffer[-500:]

            except serial.SerialException:
                logger.warning("Serial connection lost")
                self._running = False
                break
            except Exception as exc:
                logger.debug("Reader error: %s", exc)

    def _dispatch_response(self, line: str):
        try:
            resp = DeviceResponse.parse(line)
        except (ValueError, ProtocolError) as exc:
            logger.debug("Parse error: %s", exc)
            return

        with self._lock:
            event = self._pending.pop(resp.id, None)
            if event is not None:
                self._responses[resp.id] = resp
                event.set()

    def _handshake(self):
        """Verify the device is a Pclika board by exchanging a ping."""
        try:
            data = self.command("ping", timeout=HANDSHAKE_TIMEOUT)
            if not data.get("pong"):
                raise ProtocolError("Unexpected ping response")
            logger.info(
                "Device firmware v%s | board: %s",
                data.get("version", "?"),
                data.get("board", "?"),
            )
        except DeviceTimeout:
            raise DeviceNotConnected(
                "Device did not respond to handshake. "
                "Check firmware is flashed and port is correct."
            )

    def _discover_port(self) -> str:
        """Auto-discover the first connected Pclika device."""
        candidates = []
        for port_info in serial.tools.list_ports.comports():
            desc = (port_info.description or "").lower()
            vid  = port_info.vid
            # Common USB-Serial chips: CH340 (0x1A86), CP210x (0x10C4), FTDI (0x0403)
            # ESP32-S3 native USB CDC: 0x303A
            if vid in (0x1A86, 0x10C4, 0x0403, 0x303A) or "uart" in desc or "serial" in desc:
                candidates.append(port_info.device)

        if not candidates:
            raise DeviceNotConnected(
                "No Pclika device found. Connect your board via USB and try again. "
                "Tip: use --port /dev/ttyUSB0 (Linux) or --port COM3 (Windows) to specify manually."
            )

        if len(candidates) == 1:
            logger.info("Auto-detected port: %s", candidates[0])
            return candidates[0]

        # Multiple candidates: try each one
        for port in candidates:
            try:
                test = serial.Serial(port, self._baud, timeout=0.2)
                test.close()
                logger.info("Using port: %s (first responsive)", port)
                return port
            except serial.SerialException:
                continue

        raise DeviceNotConnected(
            f"Found {len(candidates)} possible ports but none responded: "
            + ", ".join(candidates)
            + "\nSpecify manually with --port."
        )
