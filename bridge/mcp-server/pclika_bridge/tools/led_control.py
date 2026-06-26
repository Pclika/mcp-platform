"""led_control — Control onboard or addressable LEDs."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

SCHEMA = {
    "name": "led_control",
    "description": (
        "Control the onboard LED or an addressable LED strip. "
        "For the onboard LED: state='on'/'off'/'blink'. "
        "For RGB LEDs: provide r, g, b values (0–255)."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {
            "state": {
                "type": "string",
                "description": "LED state: 'on', 'off', or 'blink'.",
                "enum": ["on", "off", "blink"],
            },
            "r": {"type": "integer", "description": "Red   (0–255). Only for RGB LEDs.", "minimum": 0, "maximum": 255},
            "g": {"type": "integer", "description": "Green (0–255). Only for RGB LEDs.", "minimum": 0, "maximum": 255},
            "b": {"type": "integer", "description": "Blue  (0–255). Only for RGB LEDs.", "minimum": 0, "maximum": 255},
            "brightness": {
                "type": "integer",
                "description": "Brightness 0–255. Default: 128.",
                "default": 128,
                "minimum": 0,
                "maximum": 255,
            },
            "blink_hz": {
                "type": "number",
                "description": "Blink frequency in Hz (only when state='blink'). Default: 1.0.",
                "default": 1.0,
            },
        },
        "required": ["state"],
    },
}


def led_control(transport: "SerialTransport", arguments: dict) -> str:
    params = {
        "state":      arguments.get("state", "off"),
        "r":          int(arguments.get("r", 255)),
        "g":          int(arguments.get("g", 255)),
        "b":          int(arguments.get("b", 255)),
        "brightness": int(arguments.get("brightness", 128)),
        "blink_hz":   float(arguments.get("blink_hz", 1.0)),
    }
    data = transport.command("led_control", params)
    return json.dumps(data, indent=2, ensure_ascii=False)
