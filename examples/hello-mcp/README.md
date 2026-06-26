# hello-mcp

The minimum working Pclika MCP example.

**What it does:** Blinks the onboard LED, responds to `device_info`, and logs boot messages. First proof that your board and bridge are working.

**Time to first demo:** ~10 minutes from a fresh board.

---

## Hardware

- Pclika baseboard (ESP32-S3) — or any ESP32-S3 DevKit
- USB-C cable (data capable)

No external modules required.

---

## Files

```
hello-mcp/
  main/
    main.c          ← Firmware (LED blink + MCP bridge)
    CMakeLists.txt
  CMakeLists.txt
  sdkconfig.defaults
  demo.py           ← Python demo script (calls MCP tools directly)
  README.md         ← This file
```

---

## Step 1 — Flash the firmware

```bash
# From this directory
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash monitor
```

Expected boot output:
```
I (xxx) hello_mcp: === hello-mcp v0.1.0 ===
I (xxx) hello_mcp: Seal: PCK-MMXXVI-C4A32096
I (xxx) hello_mcp: LED blinking on GPIO48
I (xxx) hello_mcp: hello-mcp ready — waiting for MCP host
```

LED should blink at 1 Hz. Press Ctrl+] to exit monitor.

---

## Step 2 — Start the MCP bridge

```bash
pip install ./bridge/mcp-server

# Auto-detect port
pclika-bridge

# Or specify port
pclika-bridge --port /dev/ttyUSB0     # Linux
pclika-bridge --port COM3             # Windows
pclika-bridge --port /dev/cu.usbserial-0001  # macOS
```

Expected output (on stderr):
```
INFO pclika.transport: Connected and handshake OK on /dev/ttyUSB0
INFO pclika.server: pclikaPlatform MCP server ready (STDIO)
```

---

## Step 3 — Run the demo script

```bash
python demo.py
```

Or with a specific port:
```bash
python demo.py --port /dev/ttyUSB0
```

Expected output:
```
============================================================
  Pclika hello-mcp demo
  PCK-MMXXVI-C4A32096
============================================================

✓ Connected on /dev/ttyUSB0

1. device_info
{
  "board_id": "AA:BB:CC:DD:EE:FF",
  "firmware": "0.1.0",
  "platform": "esp32s3",
  "seal": "PCK-MMXXVI-C4A32096",
  "capabilities": { ... }
}

2. firmware_version
{ "version": "0.1.0", "build_date": "Jun 25 2026", ... }

3. led_control — blink 3 times via MCP
  [1/3] LED ON
  [1/3] LED OFF
  ...

4. serial_log_read — last 10 device log lines
  I (123) hello_mcp: === hello-mcp v0.1.0 ===
  ...
```

---

## Step 4 — Connect Claude or Codex

Add to your MCP client config (`~/.cursor/mcp.json` or equivalent):

```json
{
  "mcpServers": {
    "pclikaPlatform": {
      "command": "pclika-bridge",
      "args": ["--port", "/dev/ttyUSB0"]
    }
  }
}
```

Then in Claude or Codex:

```
# Verify connection
device_info

# Blink LED
led_control(state="blink", r=0, g=200, b=100, blink_hz=2)

# Check logs
serial_log_read(lines=20)
```

---

## MCP Tools Available in This Example

| Tool | What it does |
|------|-------------|
| `device_info` | Board identity, firmware version, capability flags |
| `firmware_version` | Firmware version + build date + platform seal |
| `led_control` | Control onboard LED (on/off/blink + RGB color) |
| `button_read` | Read onboard BOOT button state |
| `gpio_read` | Read any GPIO pin level |
| `serial_log_read` | Get recent device log lines |

---

## Troubleshooting

**"No Pclika device found"**
- Check USB cable is data-capable (try a different cable)
- Run `pclika-bridge --list-ports` to see available ports
- Specify port manually with `--port`

**"Device did not respond to handshake"**
- Confirm firmware is flashed: `idf.py flash`
- Confirm baud rate matches: default is 115200

**LED not blinking**
- Check `LED_GPIO` in `main.c` matches your board's LED pin
- ESP32-S3 DevKitC-1: GPIO48 (RGB LED WS2812, needs different driver)
- Generic ESP32-S3: try GPIO2 or GPIO4

---

## Next Example

→ `examples/env-monitor` — Add a DHT22 sensor and OLED display
