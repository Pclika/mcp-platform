# Claude Code MCP Setup

Add both pclika MCP servers: the hardware platform bridge and the HDL bridge.

## Hardware Platform Bridge (pclikaPlatform)

```bash
# Local scope (this project only)
claude mcp add pclikaPlatform -- pclika-bridge --port auto --baud 921600

# User scope (all projects)
claude mcp add --scope user pclikaPlatform -- pclika-bridge --port auto --baud 921600
```

## HDL Bridge (pclikaHDL)

```bash
# Local scope — adjust --project to your HDL project root
claude mcp add pclikaHDL -- pclika-hdl-bridge \
  --project . \
  --device ice40up5k \
  --package sg48 \
  --freq 12

# User scope
claude mcp add --scope user pclikaHDL -- pclika-hdl-bridge \
  --project /absolute/path/to/hdl-project \
  --device ice40up5k --package sg48 --freq 12
```

## Verify

```bash
claude mcp list
```

In Claude Code session, use `/mcp` to confirm both servers are connected.

## Available Tools (pclikaPlatform)

`device_info`, `gpio_read`, `gpio_write`, `gpio_configure`,
`sensor_read`, `display_text`, `serial_log_read`,
`servo_move`, `servo_read`, `wifi_scan`, `wifi_connect`

## Available Tools (pclikaHDL)

`device_info`, `synth_run`, `synth_status`, `impl_run`, `impl_status`,
`timing_report`, `resource_usage`, `lint_report`,
`sim_run`, `sim_result`, `constraint_validate`,
`bitstream_flash`, `waveform_export`
