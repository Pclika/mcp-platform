# Troubleshooting — Pclika MCP Platform

**Seal:** `PCK-MMXXVI-C4A32096`

---

## Serial / Connection Issues

### `pclika-bridge: Serial port not found`

**Cause:** Wrong port name, or device not recognized by OS.

**Fix:**

```bash
# macOS — find the port
ls /dev/cu.*

# Linux
ls /dev/ttyUSB* /dev/ttyACM*

# Windows — check Device Manager → Ports (COM & LPT)
```

Then restart the bridge with the correct port:

```bash
pclika-bridge --port /dev/cu.usbserial-0001 --baud 115200
```

---

### `Permission denied: /dev/ttyUSB0` (Linux)

**Cause:** Current user is not in the `dialout` group.

**Fix:**

```bash
sudo usermod -aG dialout $USER
# Log out and back in, then retry
```

---

### Bridge connects but no `device_info` response

**Cause:** Firmware is not running or is stuck in boot loop.

**Check:**

```bash
idf.py monitor --port /dev/ttyUSB0
```

Look for:
- `Brownout detector was triggered` → power supply insufficient (use powered USB hub)
- `Guru Meditation Error` → firmware crash (see crash section below)
- `pclika_bridge: MCP bridge ready` → firmware is running, bridge config issue

---

### `device_info()` times out after flash

**Cause:** `idf.py monitor` is still holding the serial port open.

**Fix:** Press `Ctrl+]` to exit the monitor before starting the bridge.

---

### Bridge disconnects mid-session

**Cause:** USB cable, power fluctuation, or WDT reset.

**Fix:** Use a powered USB hub and a data-rated cable. Add `CONFIG_ESP_TASK_WDT_TIMEOUT_S=30` to `sdkconfig.defaults` if the task watchdog is firing.

---

## Firmware Issues

### `idf.py build` fails with `CMake Error: Could not find a package configuration file`

**Cause:** ESP-IDF environment not sourced.

**Fix:**

```bash
# macOS / Linux
. $HOME/esp/esp-idf/export.sh

# Windows PowerShell
C:\Espressif\frameworks\esp-idf-v5.x\export.ps1
```

---

### `fatal error: pclika_bridge.h: No such file or directory`

**Cause:** Example's `CMakeLists.txt` `REQUIRES` a component that isn't in the build path.

**Fix:** Make sure you're running `idf.py build` from inside the example directory (the one with its own `CMakeLists.txt`), not from the repo root.

---

### Firmware flashes but board immediately resets

**Cause:** Partition table mismatch. The firmware expects the `partitions.csv` defined in the project.

**Fix:**

```bash
idf.py erase_flash
idf.py build flash monitor
```

---

### `Guru Meditation Error: Core 0 panic'ed (InstrFetchProhibited)`

**Cause:** Stack overflow or null function pointer. Increase main task stack or check `pclika_bridge_register_tool` calls.

**Fix:**

In `sdkconfig.defaults`:
```
CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192
```

---

## Sensor / Driver Issues

### `sensor_read` returns `"error": "sensor not found"`

**Cause:** Sensor driver not registered in firmware, or sensor not wired.

**Check:**
1. `device_info()` → look at `capabilities` array. The sensor family must appear there.
2. Check physical wiring: SDA/SCL for I2C, correct GPIO for DHT22/one-wire.
3. Verify pull-up resistors on I2C lines (4.7 kΩ to 3.3 V).

---

### BME280 / SHT31 reads return garbage values

**Cause:** I2C address conflict or missing pull-ups.

**Fix:**
- BME280 default address: `0x76` (SDO=GND) or `0x77` (SDO=3.3V)
- SHT31 default address: `0x44` (ADDR=GND) or `0x45` (ADDR=3.3V)
- Run an I2C scanner to confirm detected addresses:

```c
// Temporary debug — scan 0x08 to 0x77
for (uint8_t addr = 0x08; addr < 0x78; addr++) {
    uint8_t dummy;
    if (i2c_master_read_from_device(I2C_NUM_0, addr, &dummy, 1, 50/portTICK_PERIOD_MS) == ESP_OK) {
        ESP_LOGI("scan", "Found device at 0x%02X", addr);
    }
}
```

---

### DHT22 always returns error

**Cause:** Timing-sensitive single-wire protocol susceptible to interrupt jitter.

**Fix:**
- Use a 10 kΩ pull-up on the data line to 3.3 V.
- Do not place DHT22 reads inside FreeRTOS tasks with very short periods. Minimum read interval: 2 seconds.
- Check GPIO selection — DHT22 doesn't work on input-only pins (GPIO 34–39 on ESP32).

---

### `env_read` returns `dew_point_c: null`

**Cause:** Dew point calculation requires both temperature and humidity to be present. If either field is `null` (pressure-only sensor), dew point cannot be computed.

**Fix:** Use BME280 or DHT22/SHT31 which provide both T and RH.

---

### VL53L0X always returns `valid: false, distance_mm: 8190`

**Cause:** Target out of range, or laser blocked.

**Fix:**
- Ensure unobstructed line of sight. Minimum reliable distance: 30 mm.
- Dark, light-absorbing surfaces reduce signal rate — check `signal_rate` value.
- Default profile range: ~1.2 m. Use `PCLIKA_VL53L0X_PROFILE_LONG_RANGE` for up to 2 m.

---

## Display Issues

### SSD1306 shows nothing after `display_text`

**Cause:** I2C address mismatch (0x3C vs 0x3D), or missing pull-ups.

**Fix:**
- Check SA0 pin on your module. SA0=GND → 0x3C, SA0=VCC → 0x3D.
- Set the correct address in firmware: `pclika_ssd1306_init(I2C_NUM_0, 0x3C, &oled)`.
- Add 4.7 kΩ pull-ups on SDA and SCL to 3.3 V.

---

### ST7789 displays only white or all-black

**Cause:** Missing `INVON` command for your specific panel, or SPI mode mismatch.

**Fix:**
- ST7789 requires **SPI Mode 3** (CPOL=1, CPHA=1). Verify in `spi_device_interface_config_t.mode = 3`.
- Some 240×240 panels need a different col/row offset. Try `col_offset=0/80` and `row_offset=0/80` combinations.
- If colours are inverted, the `INVON` command (`0x21`) handles this. It's included in `run_init()` by default.

---

## MCP Bridge (Python) Issues

### Claude says "Tool `sensor_read` not found"

**Cause:** MCP server was not registered correctly in the client config.

**Fix:**

Claude Code:
```bash
claude mcp list     # should show pclika-bridge
claude mcp remove pclika-bridge
claude mcp add pclika-bridge -- pclika-bridge --port /dev/ttyUSB0
```

Cursor: Reload the window after saving `.cursor/mcp.json`.

---

### `pyserial` import error on macOS arm64

**Fix:**

```bash
pip install --upgrade pyserial
pip install ./bridge/mcp-server
```

If using Homebrew Python:

```bash
/opt/homebrew/bin/pip3 install ./bridge/mcp-server
```

---

### Bridge runs but tool calls return timeout

**Cause:** Baud rate mismatch between firmware and bridge.

**Fix:** Firmware default baud is `115200`. If your sdkconfig uses a different rate:

```bash
pclika-bridge --port /dev/ttyUSB0 --baud 921600
```

---

### `ERROR: CRC mismatch in sensor response`

**Cause:** Electrical noise on I2C, or power fluctuation during read.

**Fix:**
- Shorten I2C wire runs (< 30 cm for 400 kHz).
- Add 100 nF decoupling capacitor near sensor VCC pin.
- Reduce I2C clock: `I2C_MASTER_FREQ_HZ 100000` in firmware config.

---

## FPGA (pclika-hdl) Issues

### `nextpnr-ice40: device not found`

**Cause:** iCEBreaker not recognized over USB.

**Fix:**

```bash
# Install udev rule
sudo cp toolchain/scripts/99-pclika-usb.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
# Unplug and replug the board
```

---

### Synthesis fails: `ERROR: Unknown module type: pclika_uart_rx`

**Cause:** IP library path not included in synthesis command.

**Fix:** Use the provided Makefile which sets `VERILOG_SRCS` correctly:

```bash
cd examples/uart-echo
make synth
```

---

## Getting help

1. Check this guide and [docs/api-reference.md](api-reference.md) first.
2. Search [GitHub Issues](https://github.com/Pclika/mcp-platform/issues) — your problem may already be solved.
3. Open a new issue with: board name, firmware example used, OS, error message, and `serial_log_read()` output.
4. Enterprise support: [pclika.com/b2b](https://pclika.com/b2b)
