"""
Pclika MCP Bridge Server — STDIO transport.

Implements the Model Context Protocol (MCP) over STDIO (JSON-RPC 2.0).
Connects to a Pclika device via USB-Serial and exposes hardware tools
to Claude, Codex, Cursor, OpenCode, and any MCP-compatible AI coding tool.
"""

from __future__ import annotations

import json
import logging
import sys
from typing import Any

from .transport import SerialTransport
from .exceptions import PclikaError, DeviceNotConnected, InvalidParameter
from .tools import (
    device_info, sensor_read, display_text, servo_move, serial_log_read,
    led_control, button_read, wifi_scan, gpio_read, gpio_write, firmware_version,
)
from .tools.device_info    import SCHEMA as SCHEMA_DEVICE_INFO
from .tools.sensor_read    import SCHEMA as SCHEMA_SENSOR_READ
from .tools.display_text   import SCHEMA as SCHEMA_DISPLAY_TEXT
from .tools.servo_move     import SCHEMA as SCHEMA_SERVO_MOVE
from .tools.serial_log_read import SCHEMA as SCHEMA_SERIAL_LOG_READ
from .tools.led_control    import SCHEMA as SCHEMA_LED_CONTROL
from .tools.button_read    import SCHEMA as SCHEMA_BUTTON_READ
from .tools.wifi_scan      import SCHEMA as SCHEMA_WIFI_SCAN
from .tools.gpio           import GPIO_READ_SCHEMA, GPIO_WRITE_SCHEMA
from .tools.firmware_version import SCHEMA as SCHEMA_FIRMWARE_VERSION

logger = logging.getLogger("pclika.server")

# ── Tool registry ─────────────────────────────────────────────────────────────

TOOLS: dict[str, tuple[dict, Any]] = {
    "device_info":      (SCHEMA_DEVICE_INFO,      device_info),
    "sensor_read":      (SCHEMA_SENSOR_READ,       sensor_read),
    "display_text":     (SCHEMA_DISPLAY_TEXT,      display_text),
    "servo_move":       (SCHEMA_SERVO_MOVE,        servo_move),
    "serial_log_read":  (SCHEMA_SERIAL_LOG_READ,   serial_log_read),
    "led_control":      (SCHEMA_LED_CONTROL,       led_control),
    "button_read":      (SCHEMA_BUTTON_READ,       button_read),
    "wifi_scan":        (SCHEMA_WIFI_SCAN,         wifi_scan),
    "gpio_read":        (GPIO_READ_SCHEMA,         gpio_read),
    "gpio_write":       (GPIO_WRITE_SCHEMA,        gpio_write),
    "firmware_version": (SCHEMA_FIRMWARE_VERSION,  firmware_version),
}

SERVER_NAME    = "pclikaPlatform"
SERVER_VERSION = "0.1.0"


# ── JSON-RPC helpers ──────────────────────────────────────────────────────────

def _ok(req_id: Any, result: Any) -> dict:
    return {"jsonrpc": "2.0", "id": req_id, "result": result}

def _err(req_id: Any, code: int, message: str, data: Any = None) -> dict:
    err: dict = {"code": code, "message": message}
    if data is not None:
        err["data"] = data
    return {"jsonrpc": "2.0", "id": req_id, "error": err}

def _send(obj: dict):
    line = json.dumps(obj, ensure_ascii=False)
    sys.stdout.write(line + "\n")
    sys.stdout.flush()


# ── Request handlers ──────────────────────────────────────────────────────────

def _handle_initialize(req_id: Any, _params: dict) -> dict:
    return _ok(req_id, {
        "protocolVersion": "2024-11-05",
        "capabilities": {"tools": {"listChanged": False}},
        "serverInfo": {"name": SERVER_NAME, "version": SERVER_VERSION},
        "instructions": (
            "Pclika MCP Bridge is connected to a Pclika hardware board. "
            "Call device_info first to confirm connection and see available capabilities. "
            "Use stable tool names: sensor_read, display_text, servo_move, "
            "led_control, button_read, wifi_scan, gpio_read, gpio_write, serial_log_read. "
            "Do not invent unsupported board layouts or ad hoc pin mappings. "
            "Treat docs/architecture/core-platform.md as the platform source of truth."
        ),
    })


def _handle_tools_list(req_id: Any, _params: dict) -> dict:
    tools = [schema for schema, _ in TOOLS.values()]
    return _ok(req_id, {"tools": tools})


def _handle_tools_call(req_id: Any, params: dict, transport: SerialTransport) -> dict:
    tool_name = params.get("name", "")
    arguments = params.get("arguments", {})

    if tool_name not in TOOLS:
        return _err(req_id, -32601, f"Tool not found: {tool_name}")

    _, fn = TOOLS[tool_name]
    try:
        result_text = fn(transport, arguments)
        return _ok(req_id, {
            "content": [{"type": "text", "text": result_text}],
            "isError": False,
        })
    except InvalidParameter as exc:
        return _ok(req_id, {
            "content": [{"type": "text", "text": f"Invalid parameter: {exc}"}],
            "isError": True,
        })
    except DeviceNotConnected as exc:
        return _ok(req_id, {
            "content": [{"type": "text", "text": f"Device not connected: {exc}"}],
            "isError": True,
        })
    except PclikaError as exc:
        return _ok(req_id, {
            "content": [{"type": "text", "text": f"Device error: {exc}"}],
            "isError": True,
        })
    except Exception as exc:
        logger.exception("Unexpected error in tool %s", tool_name)
        return _err(req_id, -32603, f"Internal error: {exc}")


# ── Main server loop ──────────────────────────────────────────────────────────

def run(transport: SerialTransport):
    """Run the MCP STDIO server. Blocks until stdin closes."""
    logger.info("pclikaPlatform MCP server ready (STDIO)")

    for raw_line in sys.stdin:
        raw_line = raw_line.strip()
        if not raw_line:
            continue

        try:
            req = json.loads(raw_line)
        except json.JSONDecodeError:
            _send(_err(None, -32700, "Parse error"))
            continue

        req_id = req.get("id")
        method = req.get("method", "")
        params = req.get("params") or {}

        # Ignore notifications (no id)
        if req_id is None and method == "notifications/initialized":
            continue

        if method == "initialize":
            _send(_handle_initialize(req_id, params))

        elif method == "tools/list":
            _send(_handle_tools_list(req_id, params))

        elif method == "tools/call":
            _send(_handle_tools_call(req_id, params, transport))

        elif method == "ping":
            _send(_ok(req_id, {}))

        else:
            _send(_err(req_id, -32601, f"Method not found: {method}"))
