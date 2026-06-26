"""button_read — Read current button state."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

SCHEMA = {
    "name": "button_read",
    "description": "Read the current state of the onboard button or an external button. Returns pressed (true/false) and press count since last read.",
    "inputSchema": {
        "type": "object",
        "properties": {
            "button_id": {
                "type": "string",
                "description": "Button identifier. 'boot' = onboard BOOT button. Default: 'boot'.",
                "default": "boot",
            },
            "reset_count": {
                "type": "boolean",
                "description": "Reset the press counter after reading. Default: false.",
                "default": False,
            },
        },
        "required": [],
    },
}


def button_read(transport: "SerialTransport", arguments: dict) -> str:
    params = {
        "button_id":   arguments.get("button_id", "boot"),
        "reset_count": bool(arguments.get("reset_count", False)),
    }
    data = transport.command("button_read", params)
    return json.dumps(data, indent=2, ensure_ascii=False)
