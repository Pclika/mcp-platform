"""
Device WebSocket relay manager + call logger.

Protocol (device → server):
  {"type":"register","api_key":"pck_xxx","board_id":"PCK-001","fw":"0.1.0"}
  {"type":"tool_result","id":"<uuid>","result":{...}}
  {"type":"tool_error","id":"<uuid>","error":"message"}

Protocol (server → device):
  {"type":"tool_call","id":"<uuid>","name":"device_info","args":{}}
"""
import asyncio
import json
import logging
import time
import uuid
from collections import deque
from typing import Any

from starlette.websockets import WebSocket

logger = logging.getLogger(__name__)

TOOL_TIMEOUT = 8.0


# ── Call Logger ────────────────────────────────────────────────────────────────
class CallLogger:
    def __init__(self, maxlen: int = 500):
        self._log: deque = deque(maxlen=maxlen)
        self._total: int = 0
        self._start: int = int(time.time())

    def record(self, key: str, tool: str, duration_ms: int, ok: bool, demo: bool = False):
        self._total += 1
        self._log.append({
            "ts": int(time.time()),
            "key": key[:12] + "…",
            "tool": tool,
            "duration_ms": duration_ms,
            "ok": ok,
            "demo": demo,
        })

    def get(self, limit: int = 100) -> list:
        entries = list(self._log)
        return entries[-limit:][::-1]  # newest first

    @property
    def total(self) -> int:
        return self._total

    @property
    def uptime_s(self) -> int:
        return int(time.time()) - self._start

    def per_minute(self, minutes: int = 60) -> list:
        now = int(time.time())
        buckets: dict[int, int] = {}
        for i in range(minutes):
            buckets[(now // 60 - i) * 60] = 0
        for entry in self._log:
            bucket = (entry["ts"] // 60) * 60
            if bucket in buckets:
                buckets[bucket] += 1
        return [{"ts": k, "count": v} for k, v in sorted(buckets.items())]

    def by_tool(self) -> dict:
        counts: dict[str, int] = {}
        for e in self._log:
            counts[e["tool"]] = counts.get(e["tool"], 0) + 1
        return dict(sorted(counts.items(), key=lambda x: -x[1]))


call_logger = CallLogger()


# ── Demo responses ──────────────────────────────────────────────────────────────
def _demo(name: str, args: dict) -> dict:
    t = int(time.time() * 1000) % 10_000_000
    sid = args.get("sensor_id", "temp_humidity")
    responses: dict[str, Any] = {
        "device_info": {
            "board_id": "PCK-DEMO-001", "fw_version": "0.1.0",
            "platform": "esp32s3", "seal": "PCK-MMXXVI-C4A32096",
            "capabilities": ["sensor", "display", "servo", "gpio", "wifi", "led"],
            "heap_free": 245760, "uptime_ms": t, "_demo": True,
        },
        "sensor_read": {
            "sensor_id": sid, "channel": args.get("channel", 0),
            "value": {"temperature": 24.5, "humidity": 58.2} if sid == "temp_humidity"
                     else (420 if sid == "light" else 1013.25),
            "unit": "%RH/°C" if sid == "temp_humidity" else ("lux" if sid == "light" else "hPa"),
            "timestamp_ms": t, "_demo": True,
        },
        "display_text": {"ok": True, "display_id": args.get("display_id", "primary"),
                         "lines_written": args.get("text", "").count("\n") + 1, "_demo": True},
        "servo_move": {"ok": True, "channel": args.get("channel", 0),
                       "angle": args.get("angle", 90), "duration_ms": 500, "_demo": True},
        "serial_log_read": {
            "lines": ["[DEMO] MCP bridge active", "[DEMO] Heap: 245760 bytes",
                      f"[DEMO] Uptime: {t}ms"],
            "count": 3, "truncated": False, "_demo": True,
        },
        "led_control": {"ok": True, "state": args.get("state", "on"), "_demo": True},
        "button_read": {"button_id": args.get("button_id", "boot"), "pressed": False,
                        "press_count": 0, "last_press_ms": -1, "_demo": True},
        "wifi_scan": {
            "networks": [
                {"ssid": "Demo-Network", "rssi": -65, "channel": 6,
                 "security": "WPA2", "bssid": "AA:BB:CC:DD:EE:FF"},
                {"ssid": "Pclika-Lab", "rssi": -72, "channel": 11,
                 "security": "WPA3", "bssid": "11:22:33:44:55:66"},
            ],
            "count": 2, "scan_time_ms": 1200, "_demo": True,
        },
        "gpio_read": {"pin": args.get("pin", 0), "level": 0,
                      "pull": args.get("pull", "none"), "_demo": True},
        "gpio_write": {"ok": True, "pin": args.get("pin", 0),
                       "level": args.get("level", 0), "_demo": True},
        "firmware_version": {
            "version": "0.1.0", "build_date": "2026-06-26", "idf_version": "5.2.0",
            "seal": "PCK-MMXXVI-C4A32096",
            "tools": ["device_info", "sensor_read", "display_text", "servo_move",
                      "serial_log_read", "led_control", "button_read", "wifi_scan",
                      "gpio_read", "gpio_write", "firmware_version"],
            "_demo": True,
        },
    }
    return responses.get(name, {"error": f"Unknown tool: {name}", "_demo": True})


# ── Device Manager ─────────────────────────────────────────────────────────────
class DeviceManager:
    def __init__(self):
        self._connections: dict[str, WebSocket] = {}
        self._pending: dict[str, asyncio.Future] = {}
        self._device_info: dict[str, dict] = {}   # api_key → {board_id, connected_at, call_count}

    def is_connected(self, api_key: str) -> bool:
        return api_key in self._connections

    def list_devices(self) -> list:
        return [
            {"key": k[:12] + "…", **v}
            for k, v in self._device_info.items()
            if k in self._connections
        ]

    async def register(self, api_key: str, ws: WebSocket, board_id: str = "unknown"):
        self._connections[api_key] = ws
        self._device_info[api_key] = {
            "board_id": board_id,
            "connected_at": int(time.time()),
            "call_count": 0,
            "last_call": None,
        }
        logger.info("Device registered: %s key=%s…", board_id, api_key[:12])
        try:
            async for raw in ws.iter_text():
                try:
                    msg = json.loads(raw)
                except json.JSONDecodeError:
                    continue
                if msg.get("type") in ("tool_result", "tool_error"):
                    call_id = msg.get("id")
                    if call_id in self._pending:
                        fut = self._pending.pop(call_id)
                        if not fut.done():
                            if msg["type"] == "tool_error":
                                fut.set_exception(RuntimeError(msg.get("error", "device error")))
                            else:
                                fut.set_result(msg.get("result", {}))
        finally:
            self._connections.pop(api_key, None)
            logger.info("Device disconnected: %s", board_id)

    async def call(self, api_key: str, tool_name: str, args: dict) -> dict:
        t0 = time.time()
        ws = self._connections.get(api_key)

        if ws is None:
            result = _demo(tool_name, args)
            duration = int((time.time() - t0) * 1000)
            call_logger.record(api_key, tool_name, duration, True, demo=True)
            return result

        call_id = str(uuid.uuid4())
        loop = asyncio.get_event_loop()
        fut: asyncio.Future = loop.create_future()
        self._pending[call_id] = fut

        try:
            await ws.send_text(json.dumps({
                "type": "tool_call", "id": call_id,
                "name": tool_name, "args": args,
            }))
            result = await asyncio.wait_for(fut, timeout=TOOL_TIMEOUT)
            ok = True
        except asyncio.TimeoutError:
            self._pending.pop(call_id, None)
            result = {"error": "device timeout", "tool": tool_name}
            ok = False
        except Exception as exc:
            self._pending.pop(call_id, None)
            result = {"error": str(exc), "tool": tool_name}
            ok = False

        duration = int((time.time() - t0) * 1000)
        call_logger.record(api_key, tool_name, duration, ok, demo=False)

        if api_key in self._device_info:
            self._device_info[api_key]["call_count"] += 1
            self._device_info[api_key]["last_call"] = int(time.time())

        return result
