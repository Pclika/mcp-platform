"""
Pclika device protocol — NDJSON over UART.

Frame format (host → device):
  {"id": "<uuid>", "cmd": "<tool_name>", "params": {...}}\n

Frame format (device → host):
  {"id": "<uuid>", "ok": true, "data": {...}}\n
  {"id": "<uuid>", "ok": false, "error": {"code": "<code>", "msg": "<message>"}}\n

Handshake:
  Host sends:   {"id": "<uuid>", "cmd": "ping", "params": {}}\n
  Device sends: {"id": "<uuid>", "ok": true, "data": {"pong": true, "version": "0.1.0"}}\n
"""

from __future__ import annotations

import json
import uuid
from dataclasses import dataclass, field
from typing import Any


# ── Frame builders ────────────────────────────────────────────────────────────

def make_command(cmd: str, params: dict[str, Any] | None = None) -> tuple[str, str]:
    """Build a command frame. Returns (frame_id, json_line)."""
    frame_id = str(uuid.uuid4())
    frame = {"id": frame_id, "cmd": cmd, "params": params or {}}
    return frame_id, json.dumps(frame, ensure_ascii=False) + "\n"


# ── Response parser ───────────────────────────────────────────────────────────

@dataclass
class DeviceResponse:
    id: str
    ok: bool
    data: dict[str, Any] = field(default_factory=dict)
    error_code: str = ""
    error_msg: str = ""

    @classmethod
    def parse(cls, line: str) -> "DeviceResponse":
        """Parse a raw NDJSON line from the device."""
        try:
            obj = json.loads(line.strip())
        except json.JSONDecodeError as exc:
            raise ValueError(f"Malformed device frame: {line!r}") from exc

        frame_id = obj.get("id", "")
        ok = bool(obj.get("ok", False))

        if ok:
            return cls(id=frame_id, ok=True, data=obj.get("data", {}))
        else:
            err = obj.get("error", {})
            return cls(
                id=frame_id,
                ok=False,
                error_code=err.get("code", "unknown"),
                error_msg=err.get("msg", ""),
            )


# ── Known device error codes ──────────────────────────────────────────────────

class DeviceErrorCode:
    NOT_READY      = "device_not_ready"
    NOT_SUPPORTED  = "tool_not_supported"
    TIMEOUT        = "timeout"
    PARAM_INVALID  = "param_invalid"
    SENSOR_MISSING = "sensor_missing"
    DISPLAY_MISSING= "display_missing"
    SERVO_MISSING  = "servo_missing"
    WIFI_ERROR     = "wifi_error"
    INTERNAL       = "internal_error"
