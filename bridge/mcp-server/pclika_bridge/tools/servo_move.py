"""servo_move — Move a servo to a target angle."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

from ..exceptions import InvalidParameter

SCHEMA = {
    "name": "servo_move",
    "description": (
        "Move a servo motor to the specified angle. "
        "Angle range: 0–180 degrees. Channel 0 is the first servo connector. "
        "Speed controls how fast the servo moves (1 = slowest, 100 = full speed)."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {
            "angle": {
                "type": "number",
                "description": "Target angle in degrees (0–180).",
                "minimum": 0,
                "maximum": 180,
            },
            "channel": {
                "type": "integer",
                "description": "Servo channel index (0-based). Default: 0.",
                "default": 0,
                "minimum": 0,
                "maximum": 15,
            },
            "speed": {
                "type": "integer",
                "description": "Movement speed (1–100). Higher = faster. Default: 100.",
                "default": 100,
                "minimum": 1,
                "maximum": 100,
            },
        },
        "required": ["angle"],
    },
}


def servo_move(transport: "SerialTransport", arguments: dict) -> str:
    angle = arguments.get("angle")
    if angle is None:
        raise InvalidParameter("angle is required")
    angle = float(angle)
    if not (0 <= angle <= 180):
        raise InvalidParameter(f"angle must be 0–180, got {angle}")

    params = {
        "angle":   angle,
        "channel": int(arguments.get("channel", 0)),
        "speed":   int(arguments.get("speed", 100)),
    }
    data = transport.command("servo_move", params)
    return json.dumps(data, indent=2, ensure_ascii=False)
