# API Reference — Pclika MCP Platform

**Schema version:** 0.3.0  
**MCP server name:** `pclikaPlatform`  
**Seal:** `PCK-MMXXVI-C4A32096`

Full JSON schemas: [`bridge/tool-schemas/`](../bridge/tool-schemas/)

---

## Base Device Tools

### `device_info`

Return board identity, firmware version, platform seal, and capability flags. Call this first to confirm connection.

**Input:** none

**Output:**

| Field | Type | Description |
|-------|------|-------------|
| `board_id` | string | Unique board ID from MAC address |
| `fw_version` | string | Firmware version, e.g. `"0.1.0"` |
| `platform` | string | Platform name, e.g. `"esp32s3"` |
| `seal` | string | Platform integrity seal |
| `capabilities` | string[] | Registered capability identifiers |
| `heap_free` | integer | Free heap in bytes |
| `uptime_ms` | integer | Device uptime in ms |

---

### `sensor_read`

Read the current value from a supported sensor module.

**Input:**

| Param | Type | Required | Description |
|-------|------|----------|-------------|
| `sensor_id` | string | ✓ | Sensor identifier: `"temp_humidity"`, `"pressure"`, `"light"`, `"imu"`, `"distance"`, `"co2"` |
| `channel` | integer | — | Channel index when multiple same-type sensors. Default: 0 |

**Output:**

| Field | Type | Description |
|-------|------|-------------|
| `sensor_id` | string | Echo of input |
| `channel` | integer | Channel used |
| `value` | number/object/array | Primary reading. Type depends on sensor. |
| `unit` | string | Unit string, e.g. `"°C"`, `"%RH"`, `"lux"`, `"mm"` |
| `timestamp_ms` | integer | Device timestamp at read |
| `raw` | any | Raw ADC or register value for diagnostics |

---

### `display_text`

Render text on the connected display (OLED or TFT).

**Input:**

| Param | Type | Default | Description |
|-------|------|---------|-------------|
| `text` | string | ✓ | Text to display. Use `\n` for line breaks. Max 256 chars. |
| `line` | integer | 0 | Starting line index (0 = top) |
| `clear` | boolean | false | Clear display before writing |
| `display_id` | string | `"primary"` | Display identifier when multiple displays connected |

**Output:** `{ ok, display_id, lines_written }`

---

### `servo_move`

Move a servo motor to a specified angle (0–180°).

**Input:**

| Param | Type | Default | Description |
|-------|------|---------|-------------|
| `angle` | number | ✓ | Target angle in degrees (0–180) |
| `channel` | integer | 0 | Servo channel index (0–15) |
| `speed` | integer | 100 | Movement speed 1–100 |

**Output:** `{ ok, channel, angle, duration_ms }`

---

### `led_control`

Control the onboard LED or addressable RGB LEDs.

**Input:**

| Param | Type | Description |
|-------|------|-------------|
| `state` | string | `"on"`, `"off"`, or `"blink"` |
| `r`, `g`, `b` | integer | RGB values 0–255 |
| `brightness` | integer | Brightness 0–255. Default: 128 |
| `blink_hz` | number | Blink frequency in Hz. Default: 1.0 |

**Output:** `{ ok, state }`

---

### `button_read`

Read the current state of the onboard or external button.

**Input:**

| Param | Type | Default | Description |
|-------|------|---------|-------------|
| `button_id` | string | `"boot"` | Button identifier |
| `reset_count` | boolean | false | Reset press counter after read |

**Output:**

| Field | Type | Description |
|-------|------|-------------|
| `button_id` | string | — |
| `pressed` | boolean | True if currently pressed |
| `press_count` | integer | Total presses since last reset |
| `last_press_ms` | integer | ms since boot when last pressed. -1 if never. |

---

### `wifi_scan`

Scan for nearby Wi-Fi access points.

**Input:**

| Param | Type | Default |
|-------|------|---------|
| `max_results` | integer | 20 |

**Output:** `{ networks[], count, scan_time_ms }`

Each network object: `{ ssid, rssi, channel, security, bssid }`

---

### `gpio_read`

Read the digital level of a GPIO pin.

**Input:** `pin` (integer, required), `pull` (`"up"` / `"down"` / `"none"`, default `"none"`)

**Output:** `{ pin, level (0 or 1), pull }`

---

### `gpio_write`

Set a GPIO pin to a digital output level.

**Input:** `pin` (integer, required), `level` (0 or 1, required)

**Output:** `{ ok, pin, level }`

---

### `serial_log_read`

Return recent serial log output from the connected device.

**Input:** `lines` (integer, default 50, max 500), `filter` (string, optional substring filter)

**Output:** `{ lines[], count, truncated }`

---

### `firmware_version`

Return firmware version, build date, and supported tool list.

**Output:** `{ version, build_date, idf_version, seal, tools[] }`

---

## Sensor Extension Tools

Schema file: [`bridge/tool-schemas/sensor-tools.json`](../bridge/tool-schemas/sensor-tools.json)

### `env_read`

Read temperature, humidity, and optionally pressure from an environmental sensor (DHT22, SHT31, BME280).

**Input:** `sensor_id` (auto-detect by default), `channel`

**Output:** `{ temperature_c, humidity_pct, pressure_hpa, dew_point_c, sensor_id, timestamp_ms }`

---

### `imu_read`

Read 6-axis IMU data (accelerometer + gyroscope) from MPU6050 or compatible.

**Input:** `channel`, `include_temp` (boolean, default false)

**Output:** `{ accel: {x,y,z}, gyro: {x,y,z}, temp_c, timestamp_ms }`

Units: accel in m/s², gyro in °/s.

---

### `distance_read`

Read distance from a ToF sensor (VL53L0X).

**Input:** `channel`, `mode` (`"single"` / `"continuous"`, default `"single"`)

**Output:** `{ distance_mm, valid, signal_rate, timestamp_ms }`

---

### `light_read`

Read ambient light level from BH1750.

**Input:** `channel`, `resolution` (`"high"` / `"low"` / `"auto"`, default `"auto"`)

**Output:** `{ lux, raw, resolution, timestamp_ms }`

---

### `co2_read`

Read CO₂ concentration from SCD40/SCD41 or SGP41.

**Output:** `{ co2_ppm, temperature_c, humidity_pct, voc_index, timestamp_ms }`

---

## Motion Tools

Schema file: [`bridge/tool-schemas/motion-tools.json`](../bridge/tool-schemas/motion-tools.json)

### `servo_read`

Read current angle and motion state of a servo channel.

**Output:** `{ channel, angle, target_angle, moving, pulse_us }`

---

### `servo_sweep`

Sweep a servo back and forth between two angles continuously.

**Input:** `channel`, `angle_min` (default 0), `angle_max` (default 180), `speed` (1–100, default 50)

**Output:** `{ ok, channel, mode: "sweep" }`

Call `servo_stop` to halt.

---

### `servo_stop`

Stop an active servo sweep and hold position.

**Output:** `{ ok, channel, angle }`

---

### `motor_set`

Set DC motor speed and direction (TB6612FNG or equivalent).

**Input:** `channel` (0 or 1), `speed` (0–255), `direction` (`"forward"` / `"reverse"` / `"brake"` / `"coast"`)

**Output:** `{ ok, channel, speed, direction }`

---

### `stepper_move`

Move a stepper motor a specified number of steps (A4988/DRV8825).

**Input:** `steps` (positive = forward, negative = reverse), `speed_rpm` (default 60), `microstepping` (1/2/4/8/16/32)

**Output:** `{ ok, steps_moved, duration_ms, position_steps }`

---

## Vision Tools

Schema file: [`bridge/tool-schemas/vision-tools.json`](../bridge/tool-schemas/vision-tools.json)

### `camera_capture`

Capture a JPEG frame from OV2640 or compatible camera module.

**Input:** `resolution` (`QVGA`/`VGA`/`SVGA`/`XGA`/`HD`/`SXGA`/`UXGA`, default `VGA`), `quality` (10–63, default 20), `format` (`"jpeg"` / `"rgb565"`)

**Output:** `{ ok, width, height, size_bytes, base64, timestamp_ms }`

---

### `camera_status`

Query camera module status and current settings.

**Output:** `{ initialized, sensor_model, resolution, quality, brightness, saturation, flip_vertical, flip_horizontal, fps }`

---

### `camera_configure`

Configure camera sensor parameters.

**Input:** `resolution`, `quality`, `brightness` (-2–2), `contrast` (-2–2), `saturation` (-2–2), `awb`, `aec`, `flip_vertical`, `flip_horizontal`

**Output:** `{ ok, applied }`

---

### `image_read`

Read a previously saved image from device flash or SD card.

**Input:** `filename` (required), `offset`, `length`

**Output:** `{ ok, filename, size_bytes, base64, more }`

---

## Industrial Tools

Schema file: [`bridge/tool-schemas/industrial-tools.json`](../bridge/tool-schemas/industrial-tools.json)

### `modbus_read`

Read Modbus registers (RTU over RS-485 or TCP).

**Input:**

| Param | Type | Required | Description |
|-------|------|----------|-------------|
| `slave_id` | integer | ✓ | Modbus slave address 1–247 |
| `function` | string | ✓ | `"holding_registers"`, `"input_registers"`, `"coils"`, `"discrete_inputs"` |
| `address` | integer | ✓ | Register start address (0-based) |
| `count` | integer | ✓ | Number of registers/coils to read (1–125) |
| `transport` | string | — | `"rtu"` (default) or `"tcp"` |
| `timeout_ms` | integer | — | Default: 500 |

**Output:** `{ ok, slave_id, address, values[], count }`

---

### `modbus_write`

Write Modbus holding registers or coils.

**Input:** `slave_id`, `function` (`"holding_registers"` / `"coils"`), `address`, `values[]`, `transport`

**Output:** `{ ok, slave_id, address, values_written }`

---

### `relay_set`

Control relay module outputs (up to 8 channels).

**Input:**

| Param | Type | Description |
|-------|------|-------------|
| `channel` | integer | 0–7 for single channel; -1 for all channels |
| `state` | string | `"on"`, `"off"`, `"toggle"` |
| `pulse_ms` | integer | Optional: pulse on for N ms then restore |

**Output:** `{ ok, channel, state, all_states[] }`

---

### `digital_io_read`

Read a bank of digital inputs simultaneously.

**Input:** `bank` (default 0), `channel` (optional — read single channel only)

**Output:** `{ bank, levels[], mask }`

---

### `adc_read_multi`

Read multiple ADC channels simultaneously.

**Input:**

| Param | Type | Description |
|-------|------|-------------|
| `channels` | integer[] | ADC channel indices to read |
| `adc_source` | string | `"internal"` (default), `"mcp3204"`, `"ads1115"` |
| `vref` | number | Reference voltage in V. Default: 3.3 |
| `bits` | integer | ADC resolution: 10, 12, or 16 |

**Output:** `{ readings[{channel, raw, voltage}], adc_source, vref, timestamp_ms }`

---

## Error responses

All tools return a consistent error format on failure:

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32000,
    "message": "Sensor not found: temp_humidity",
    "data": { "sensor_id": "temp_humidity" }
  }
}
```

Standard error codes follow JSON-RPC 2.0. Pclika-specific codes start at `-32000`.

---

## Protocol

The bridge communicates over **NDJSON / JSON-RPC 2.0** over USB-serial UART. Each line is a complete JSON object. The device side uses the same protocol in both directions.

Request format:
```json
{"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":"sensor_read","arguments":{"sensor_id":"temp_humidity"}}}
```

Response format:
```json
{"jsonrpc":"2.0","id":1,"result":{"content":[{"type":"text","text":"{\"value\":24.3,\"unit\":\"°C\"}"}]}}
```

Full protocol specification: [`bridge/server-instructions.md`](../bridge/server-instructions.md)
