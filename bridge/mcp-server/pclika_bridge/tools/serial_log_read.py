"""serial_log_read — Return recent log output from the device."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

SCHEMA = {
    "name": "serial_log_read",
    "description": (
        "Return recent serial log output from the connected device. "
        "Useful for inspecting runtime state, debugging errors, "
        "and confirming that operations completed successfully."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {
            "lines": {
                "type": "integer",
                "description": "Number of recent log lines to return. Default: 50.",
                "default": 50,
                "minimum": 1,
                "maximum": 500,
            },
            "filter": {
                "type": "string",
                "description": "Optional substring filter — only return lines containing this string.",
                "default": "",
            },
        },
        "required": [],
    },
}


def serial_log_read(transport: "SerialTransport", arguments: dict) -> str:
    n      = int(arguments.get("lines", 50))
    filt   = arguments.get("filter", "")
    lines  = transport.get_log_lines(n)

    if filt:
        lines = [l for l in lines if filt in l]

    result = {
        "lines":  lines,
        "count":  len(lines),
        "filter": filt or None,
    }
    return json.dumps(result, indent=2, ensure_ascii=False)
