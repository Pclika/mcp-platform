"""device_info — Return board identity and capability flags."""

from __future__ import annotations
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport


SCHEMA = {
    "name": "device_info",
    "description": (
        "Return the connected board's identity, firmware version, and capability flags. "
        "Call this first to confirm connection and discover what tools are available."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {},
        "required": [],
    },
}


def device_info(transport: "SerialTransport", arguments: dict) -> str:
    """Execute device_info tool. Returns JSON string."""
    import json
    data = transport.command("device_info")
    return json.dumps(data, indent=2, ensure_ascii=False)
