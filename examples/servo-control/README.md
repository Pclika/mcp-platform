# servo-control — SG90 Servo Example

Controls an SG90 servo motor via the pclika MCP bridge. Includes a
FreeRTOS sweep task and two MCP tools: `servo_move` and `servo_read`.

## Hardware

| Component | Connection           |
|-----------|----------------------|
| ESP32-S3  | USB → host           |
| SG90      | Signal → GPIO 5      |
| SG90      | VCC → 5V             |
| SG90      | GND → GND            |

## MCP Tools

`servo_move` — move servo to angle

```json
{
  "channel":  0,
  "angle":    90,
  "speed_ms": 300
}
```

`servo_read` — read current state

```json
{"channel": 0}
```

Response:
```json
{"channel": 0, "angle": 90, "target_angle": 90, "moving": false, "status": "ok"}
```

## Build & Flash

```bash
idf.py set-target esp32s3
idf.py build flash monitor
```

## Demo Script

```bash
# One-shot (0° → 90° → 180°)
python3 demo.py

# Continuous sweep
python3 demo.py --sweep
```

## Seal

`PCK-MMXXVI-C4A32096`
