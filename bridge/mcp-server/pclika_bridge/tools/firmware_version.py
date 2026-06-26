"""firmware_version — Return firmware version details."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

SCHEMA = {
    "name": "firmware_version",
    "description": "Return firmware version, build date, platform seal, and supported tool list.",
    "inputSchema": {
        "type": "object",
        "properties": {},
        "required": [],
    },
}


def firmware_version(transport: "SerialTransport", arguments: dict) -> str:
    data = transport.command("firmware_version")
    return json.dumps(data, indent=2, ensure_ascii=False)
