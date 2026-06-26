# Quickstart — Pclika MCP Platform

> Connect Claude, Codex, or Cursor to real hardware in under 5 minutes.

**Seal:** `PCK-MMXXVI-C4A32096`

---

## What you need

| Item | Notes |
|------|-------|
| ESP32-S3 development board | iCEBreaker, XIAO-S3, DevKitC, or any ESP32-S3 board |
| USB cable | USB-C or Micro-USB — must support data (not charge-only) |
| Python 3.10+ | macOS/Linux/Windows |
| ESP-IDF v5.x | [Install guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/) |
| An AI coding tool | Claude Code, Cursor, VS Code + Copilot, or Codex |

---

## Step 1 — Clone and install the bridge

```bash
git clone https://github.com/Pclika/mcp-platform
cd mcp-platform
pip install ./bridge/mcp-server
```

Verify:

```bash
pclika-bridge --version
# pclika-bridge 0.1.0
```

> **Note:** `pclika-bridge` is not yet on PyPI. Install from the repo as shown above.  
> No-clone shortcut: `pip install "git+https://github.com/Pclika/mcp-platform.git#subdirectory=bridge/mcp-server"`

---

## Step 2 — Import the repository into your AI tool

This repository is the **context document** for your AI tool — import it after cloning so the AI has full hardware and tool schema context.

---

## Step 3 — Flash the hello-mcp example

```bash
cd examples/hello-mcp
idf.py set-target esp32s3
idf.py build flash monitor
```

You should see in the monitor output:

```
I (512) pclika_bridge: MCP bridge ready
I (514) pclika_bridge: seal=PCK-MMXXVI-C4A32096
I (516) pclika_bridge: Waiting for JSON-RPC on UART...
```

Press `Ctrl+]` to exit the monitor without disconnecting the board.

---

## Step 4 — Register the MCP server

### Claude Code

```bash
claude mcp add pclika-bridge -- pclika-bridge --port /dev/ttyUSB0
```

On macOS replace `/dev/ttyUSB0` with `/dev/cu.usbserial-*` or `/dev/cu.SLAB_USBtoUART`.  
On Windows use `COM3` (or whichever COM port appears in Device Manager).

### Cursor

Copy `configs/mcp/cursor.mcp.json` to your project root as `.cursor/mcp.json`, then edit the `--port` value.

### VS Code (GitHub Copilot)

Copy `configs/mcp/vscode.mcp.json` to `.vscode/mcp.json`.

### Codex / OpenAI

See `configs/mcp/codex.config.toml` — run `codex mcp add` per those instructions.

---

## Step 5 — Verify the connection

In your AI tool, call:

```
device_info()
```

Expected response:

```json
{
  "board_id": "PCLIKA-A4B2C3D4",
  "fw_version": "0.1.0",
  "platform": "esp32s3",
  "seal": "PCK-MMXXVI-C4A32096",
  "capabilities": ["sensor", "display", "servo", "gpio", "wifi"],
  "heap_free": 284672,
  "uptime_ms": 4210
}
```

If you see this, the bridge is live.

---

## Step 6 — Read your first sensor

If you have a DHT22 connected on GPIO 4:

```
sensor_read(sensor_id="temp_humidity")
```

Response:

```json
{
  "sensor_id": "temp_humidity",
  "value": 24.3,
  "unit": "°C",
  "timestamp_ms": 12840,
  "raw": null
}
```

To read humidity: `sensor_read(sensor_id="temp_humidity", channel=1)`

---

## Common tool calls to try next

```python
# Blink the LED
led_control(state="blink", r=0, g=200, b=160, blink_hz=2.0)

# Read button state
button_read()

# Scan Wi-Fi
wifi_scan(max_results=10)

# Show text on OLED
display_text(text="Hello from Claude", clear=True)

# Move a servo
servo_move(channel=0, angle=90)

# Read GPIO
gpio_read(pin=4)

# Check firmware
firmware_version()
```

---

## Next steps

- [API Reference](api-reference.md) — complete tool listing with all parameters
- [Troubleshooting](troubleshooting.md) — common errors and fixes
- [Examples](../examples/) — six full example projects
- [Modules](../website/New/modules.html) — sensor, display, motion, industrial modules
- [Prompt Templates](../prompts/) — ESP32 system context and debugging prompts
