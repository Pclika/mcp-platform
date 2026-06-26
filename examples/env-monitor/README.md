# env-monitor

Temperature + humidity monitoring via MCP. Reads a DHT22 sensor and writes results to an SSD1306 OLED — all queryable from Claude, Codex, or Cursor through the Pclika MCP bridge.

---

## Hardware

### Required

| Component | Spec |
|-----------|------|
| Pclika baseboard (ESP32-S3) | or any ESP32-S3 DevKit |
| DHT22 sensor | AM2302, temperature + humidity |
| SSD1306 OLED | 128×64, I2C |
| USB-C cable (data) | |

### Wiring Diagram

```
ESP32-S3 DevKit                DHT22
─────────────────              ──────────────────
GPIO4  ────────────────────── DATA (pin 2)
3.3V   ────────────────────── VCC  (pin 1)
GND    ────────────────────── GND  (pin 4)
                               pin 3: NC
         4.7kΩ pull-up:
         DATA ──┤4.7k├── 3.3V

ESP32-S3 DevKit                SSD1306 OLED (I2C)
─────────────────              ──────────────────
GPIO8  ────────────────────── SDA
GPIO9  ────────────────────── SCL
3.3V   ────────────────────── VCC
GND    ────────────────────── GND
```

**Important:**
- DHT22 DATA line needs a 4.7kΩ pull-up resistor to 3.3V
- SSD1306 default I2C address: `0x3C` (some modules use `0x3D`, check your module)
- GPIO4 and GPIO8/9 can be changed in `main.c` if your board uses different pins

---

## Files

```
env-monitor/
  main/
    main.c          ← Firmware (DHT22 + SSD1306 + MCP bridge)
    CMakeLists.txt
  CMakeLists.txt
  demo.py           ← Python demo (sensor_read + display_text via MCP)
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
I (xxx) env_monitor: === env-monitor v0.1.0 ===
I (xxx) env_monitor: Seal: PCK-MMXXVI-C4A32096
I (xxx) env_monitor: DHT22 → GPIO4
I (xxx) env_monitor: SSD1306 OLED → SDA=GPIO8 SCL=GPIO9
I (xxx) env_monitor: env-monitor ready
```

The OLED will show:
```
Pclika
env-monitor
v0.1.0
```

After 5 seconds, the auto-display task updates with live readings:
```
Temp: 24.5 C
Hum:  58.3 %
```

Press `Ctrl+]` to exit monitor.

---

## Step 2 — Start the MCP bridge

```bash
pip install ./bridge/mcp-server

# Auto-detect port
pclika-bridge

# Or specify port
pclika-bridge --port /dev/ttyUSB0     # Linux
pclika-bridge --port COM3             # Windows
pclika-bridge --port /dev/cu.usbmodem001  # macOS
```

---

## Step 3 — Run the demo script

```bash
# Single read
python demo.py

# Poll every 5 seconds
python demo.py --loop 5
```

Expected output:
```
============================================================
  Pclika env-monitor demo
  PCK-MMXXVI-C4A32096
============================================================

✓ Connected on /dev/ttyUSB0

1. device_info
  Board ID  : AA:BB:CC:DD:EE:FF
  Firmware  : 0.1.0
  has_sensor: True
  has_display: True

2. sensor_read(sensor_id="temp_humidity")
  ✓ 24.5 celsius  /  58.3 %RH

3. display_text — update OLED
  → OLED line 0: "Temp: 24.5 C"
  → OLED line 1: "Hum:  58.3 %"

4. serial_log_read — last 15 device log lines
  I env_monitor: env: 24.5°C  58.3%RH
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
# Read temperature and humidity
sensor_read(sensor_id="temp_humidity")

# Display custom text on OLED
display_text(text="Claude says hi", line=0, clear=true)
display_text(text="Hello hardware!", line=1)

# Check device and logs
device_info
serial_log_read(lines=20)
```

### Example AI prompt

> "Read the temperature and humidity from the sensor. If temperature is above 28°C, display a warning on the OLED. Otherwise show the readings in a friendly format."

Claude will call `sensor_read`, evaluate the result, then call `display_text` with appropriate content — all automatically.

---

## MCP Tools Available

| Tool | Parameters | What it does |
|------|-----------|-------------|
| `sensor_read` | `sensor_id="temp_humidity"` | Returns temp (celsius) + humidity (%RH) |
| `display_text` | `text`, `line`, `clear` | Write text to OLED line |
| `device_info` | — | Board identity + capability flags |
| `firmware_version` | — | Version + build date + seal |
| `serial_log_read` | `lines`, `filter` | Recent device log output |
| `gpio_read` | `pin` | Read GPIO level |

---

## Customization

### Change DHT22 pin
In `main/main.c`:
```c
#define DHT22_GPIO  4   // ← change this
```

### Change OLED I2C pins
```c
#define I2C_SDA_GPIO  8   // ← change this
#define I2C_SCL_GPIO  9   // ← change this
```

### Change OLED I2C address
```c
#define OLED_I2C_ADDR  0x3C   // some modules use 0x3D
```

### Register additional sensors
In `app_main`, after `pclika_sensor_init()`:
```c
pclika_sensor_register(&MY_SENSOR_DESC);
```
Then call `sensor_read(sensor_id="my_sensor_id")` via MCP.

---

## Troubleshooting

**Sensor read fails / timeout**
- Check DHT22 wiring — DATA pin needs 4.7kΩ pull-up
- DHT22 needs at least 2 seconds between reads (throttled automatically)
- First read after power-on may fail — try again in a moment

**OLED not displaying**
- Check I2C address: run an I2C scanner sketch to find your module's address
- Try swapping SDA/SCL if no output
- Confirm 3.3V supply (5V can damage ESP32 I2C pins)

**"No Pclika device found"**
- Run `pclika-bridge --list-ports`
- Check USB cable is data-capable
- Confirm firmware is flashed

---

## Previous Example

← `examples/hello-mcp` — LED blink and basic MCP proof of concept

## Next Example

→ `examples/servo-control` — Control a servo motor via `servo_move` MCP tool
