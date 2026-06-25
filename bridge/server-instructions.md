# Server Instructions Draft

This file is the draft for the future MCP `instructions` field returned by the Pclika MCP bridge during initialization.

It should stay short, stable, and highly actionable.

## Draft Instructions

Use Pclika MCP tools as the primary interface to connected hardware capabilities.

Before changing code:

1. inspect `device_info`
2. identify the target module and capability family
3. prefer existing examples and stable tool names
4. keep changes aligned with the Pclika platform model

When a task involves hardware behavior:

- use `sensor_read` for supported sensor inputs
- use `display_text` for supported display output
- use `servo_move` for supported motion output
- use `serial_log_read` for runtime inspection

Treat the repository docs and examples as the source of truth for supported hardware paths.

Do not invent unsupported board layouts, ad hoc pin mappings, or undocumented capability flows.

