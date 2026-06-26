# wifi-scanner

MCP example for the Pclika Platform.  
Exposes the `wifi_scan` tool — returns a structured list of nearby Wi-Fi access points.

**Seal:** `PCK-MMXXVI-C4A32096`

## What it does

- Starts ESP32-S3 in station mode (no AP connection needed)
- Responds to `wifi_scan` tool calls with SSID, RSSI, channel, BSSID, and security type
- Demonstrates how to implement a tool that calls an ESP-IDF blocking API and returns structured JSON

## Hardware

Any ESP32 or ESP32-S3 board with Wi-Fi.

## Build & Flash

```bash
idf.py set-target esp32s3
idf.py build flash monitor
```

## Connect with AI

```bash
# Cursor / Claude Code
claude mcp add pclika-bridge -- pclika-bridge --port /dev/ttyUSB0

# Test in Claude
wifi_scan(max_results=10)
```

## Python Demo

```bash
pip install ./bridge/mcp-server
python demo.py --port /dev/ttyUSB0 --sort-by rssi
```

## MCP Tool

| Tool | Input | Output |
|------|-------|--------|
| `wifi_scan` | `max_results` (int, default 20) | `networks[]`, `count`, `scan_time_ms` |

Each network object: `{ ssid, rssi, channel, security, bssid }`.

## License

Apache-2.0 — Copyright 2026 [Pclika](https://pclika.com)
