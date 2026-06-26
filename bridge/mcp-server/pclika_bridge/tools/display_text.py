"""display_text — Render text on a connected display module."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

from ..exceptions import InvalidParameter

SCHEMA = {
    "name": "display_text",
    "description": (
        "Render a text string on the connected display (OLED or TFT). "
        "Supports multi-line output and optional screen clear before writing."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {
            "text": {
                "type": "string",
                "description": "Text to display. Use '\\n' for line breaks.",
                "maxLength": 256,
            },
            "line": {
                "type": "integer",
                "description": "Starting line index (0 = top). Default: 0.",
                "default": 0,
                "minimum": 0,
            },
            "clear": {
                "type": "boolean",
                "description": "Clear the display before writing. Default: false.",
                "default": False,
            },
            "display_id": {
                "type": "string",
                "description": "Display identifier when multiple displays are connected. Default: 'primary'.",
                "default": "primary",
            },
        },
        "required": ["text"],
    },
}


def display_text(transport: "SerialTransport", arguments: dict) -> str:
    text = arguments.get("text")
    if text is None:
        raise InvalidParameter("text is required")

    params = {
        "text":       str(text)[:256],
        "line":       int(arguments.get("line", 0)),
        "clear":      bool(arguments.get("clear", False)),
        "display_id": arguments.get("display_id", "primary"),
    }
    data = transport.command("display_text", params)
    return json.dumps(data, indent=2, ensure_ascii=False)
