"""wifi_scan — Scan for nearby Wi-Fi access points."""

from __future__ import annotations
import json
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..transport import SerialTransport

SCHEMA = {
    "name": "wifi_scan",
    "description": (
        "Scan for nearby Wi-Fi access points and return a list of networks "
        "with SSID, signal strength (RSSI), channel, and security type."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {
            "max_results": {
                "type": "integer",
                "description": "Maximum number of networks to return. Default: 20.",
                "default": 20,
                "minimum": 1,
                "maximum": 50,
            },
        },
        "required": [],
    },
}


def wifi_scan(transport: "SerialTransport", arguments: dict) -> str:
    params = {"max_results": int(arguments.get("max_results", 20))}
    data = transport.command("wifi_scan", params, timeout=15.0)  # scan takes longer
    return json.dumps(data, indent=2, ensure_ascii=False)
