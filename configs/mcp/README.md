# MCP Configuration Templates

This directory contains starter templates for connecting MCP-compatible coding tools to the future Pclika MCP bridge.

## Status

These are starter templates, not active production configs.

They exist to standardize:

- server naming
- configuration layout
- expected activation pattern

The actual bridge implementation command or server endpoint still needs to be finalized.

## Standard Server Name

Use:

- `pclikaPlatform`

## Included Templates

- `codex.config.toml`
- `claude-code.commands.md`
- `cursor.mcp.json`
- `vscode.mcp.json`
- `opencode.jsonc`

## Activation Rule

Once the bridge is implemented:

1. replace the placeholder command or endpoint
2. set required environment variables if any
3. test `device_info`
4. test one example capability such as `sensor_read`

## Related Docs

- [../../docs/software/standard-invocation.md](../../docs/software/standard-invocation.md)
- [../../bridge/server-instructions.md](../../bridge/server-instructions.md)
