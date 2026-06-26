"""gpio_read / gpio_write — Read or write GPIO pins."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

from ..exceptions import InvalidParameter

GPIO_READ_SCHEMA = {
    "name": "gpio_read",
    "description": "Read the current digital level (HIGH/LOW) of a GPIO pin.",
    "inputSchema": {
        "type": "object",
        "properties": {
            "pin": {
                "type": "integer",
                "description": "GPIO pin number (board-specific). Use device_info to see available pins.",
                "minimum": 0,
            },
            "pull": {
                "type": "string",
                "description": "Internal pull resistor: 'up', 'down', or 'none'. Default: 'none'.",
                "enum": ["up", "down", "none"],
                "default": "none",
            },
        },
        "required": ["pin"],
    },
}

GPIO_WRITE_SCHEMA = {
    "name": "gpio_write",
    "description": "Set the digital output level of a GPIO pin to HIGH or LOW.",
    "inputSchema": {
        "type": "object",
        "properties": {
            "pin": {
                "type": "integer",
                "description": "GPIO pin number.",
                "minimum": 0,
            },
            "level": {
                "type": "integer",
                "description": "Output level: 1 (HIGH) or 0 (LOW).",
                "enum": [0, 1],
            },
        },
        "required": ["pin", "level"],
    },
}


def gpio_read(transport: "SerialTransport", arguments: dict) -> str:
    pin = arguments.get("pin")
    if pin is None:
        raise InvalidParameter("pin is required")
    params = {"pin": int(pin), "pull": arguments.get("pull", "none")}
    data = transport.command("gpio_read", params)
    return json.dumps(data, indent=2, ensure_ascii=False)


def gpio_write(transport: "SerialTransport", arguments: dict) -> str:
    pin   = arguments.get("pin")
    level = arguments.get("level")
    if pin is None:
        raise InvalidParameter("pin is required")
    if level is None:
        raise InvalidParameter("level is required (0 or 1)")
    params = {"pin": int(pin), "level": int(level)}
    data = transport.command("gpio_write", params)
    return json.dumps(data, indent=2, ensure_ascii=False)
