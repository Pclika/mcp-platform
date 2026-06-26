"""sensor_read — Read value from a connected sensor module."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

from ..exceptions import InvalidParameter

SCHEMA = {
    "name": "sensor_read",
    "description": (
        "Read the current value from a supported sensor module. "
        "Common sensor_id values: 'temp_humidity' (DHT22/SHT31), "
        "'pressure' (BME280), 'light' (BH1750), 'imu' (MPU6050), "
        "'distance' (VL53L0X), 'co2' (SCD41). "
        "Returns value, unit, and timestamp."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {
            "sensor_id": {
                "type": "string",
                "description": "Sensor identifier. Use device_info to see available sensors.",
            },
            "channel": {
                "type": "integer",
                "description": "Sensor channel index when multiple sensors of same type are connected. Default: 0.",
                "default": 0,
            },
        },
        "required": ["sensor_id"],
    },
}


def sensor_read(transport: "SerialTransport", arguments: dict) -> str:
    sensor_id = arguments.get("sensor_id")
    if not sensor_id:
        raise InvalidParameter("sensor_id is required")

    params = {
        "sensor_id": sensor_id,
        "channel": arguments.get("channel", 0),
    }
    data = transport.command("sensor_read", params)
    return json.dumps(data, indent=2, ensure_ascii=False)
