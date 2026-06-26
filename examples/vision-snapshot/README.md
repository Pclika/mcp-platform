# vision-snapshot

MCP example for the Pclika Platform.  
Captures JPEG frames from an OV2640 camera and exposes them via MCP tools.

**Seal:** `PCK-MMXXVI-C4A32096`

## What it does

- Initialises the OV2640 camera via `esp_camera`
- Exposes three MCP tools: `camera_capture`, `camera_status`, `camera_configure`
- Returns frames as Base64-encoded JPEG in tool responses

## Hardware

- ESP32-S3 board with integrated OV2640 (Seeed XIAO-S3 Sense, AI-Thinker ESP32-CAM, M5Camera)
- Default pin config targets **XIAO-S3 Sense** — override via `sdkconfig` for other boards

## Build & Flash

```bash
idf.py set-target esp32s3
idf.py build flash monitor
```

## Connect with AI

```bash
claude mcp add pclika-bridge -- pclika-bridge --port /dev/ttyUSB0

# In Claude — capture a frame
camera_capture(resolution="VGA", quality=20)

# Configure camera
camera_configure(brightness=1, flip_vertical=true)
```

## Python Demo

```bash
pip install ./bridge/mcp-server
python demo.py --port /dev/ttyUSB0 --resolution HD --output frame.jpg

# Capture 10 frames continuously
python demo.py --port /dev/ttyUSB0 --continuous 10
```

## MCP Tools

| Tool | Key Parameters | Returns |
|------|----------------|---------|
| `camera_capture` | `resolution`, `quality`, `format` | `base64` JPEG, `width`, `height`, `size_bytes` |
| `camera_status`  | — | `initialized`, `sensor_model`, `resolution`, `fps` |
| `camera_configure` | `brightness`, `contrast`, `awb`, `flip_*` | `ok`, `applied` |

## License

Apache-2.0 — Copyright 2026 [Pclika](https://pclika.com)
