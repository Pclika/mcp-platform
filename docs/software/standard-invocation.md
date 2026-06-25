# Standard Invocation Guide

## Goal

Define one standard way to import and use the Pclika MCP Platform across MCP-compatible coding tools such as Claude Code, Codex, Cursor, OpenCode, and similar clients.

This guide is the canonical reference for how the repository should be read, how the future Pclika MCP bridge should be connected, and how developers or AI tools should approach capability selection.

## Scope

This guide covers:

- repository entry order
- standard capability selection flow
- standard MCP tool naming
- build and flash interaction pattern
- client configuration templates

## Important Current Status

The repository already defines the platform structure, tool schema direction, and usage pattern.

The first runnable Pclika MCP bridge implementation is not yet included in this repository.

That means:

- the invocation model in this guide is the standard contract
- the config files under `configs/mcp/` are starter templates
- once the bridge command or endpoint is finalized, those templates can be activated with minimal changes

## Standard Entry Order

Any developer or AI coding tool should begin in this order:

1. `README.md`
2. `AGENTS.md`
3. `docs/README.md`
4. `docs/architecture/core-platform.md`
5. `docs/hardware/base-platform.md`
6. `docs/hardware/extension-modules.md`
7. the closest example under `examples/`

## Standard Development Flow

### Step 1. Import the Repository

Import the repository into an MCP-compatible coding tool.

Supported target clients include:

- Claude Code
- Codex
- Cursor
- OpenCode

### Step 2. Read Platform Context

The client should inspect the platform docs before attempting code generation or hardware-specific behavior.

### Step 3. Choose a Capability Path

The user or AI tool should choose one capability family first.

Supported first-class paths:

- sensor
- display
- servo or motion
- vision
- industrial communication
- connectivity

### Step 4. Bind Board, Module, and Runtime

For each task, identify:

- platform family
- target board
- attached module
- runtime capability
- matching MCP tool

### Step 5. Use Stable Tool Names

The standard first tool set is:

- `device_info`
- `led_control`
- `button_read`
- `sensor_read`
- `display_text`
- `servo_move`
- `wifi_scan`
- `serial_log_read`

Additional tools can be added later, but these names should remain stable.

### Step 6. Build, Flash, and Monitor

The standard local development actions are:

- `build`
- `flash`
- `monitor`

The future bridge should expose or coordinate these actions consistently across supported clients.

### Step 7. Iterate from Examples

Each new feature should begin from:

- an existing example
- a clear capability path
- a stable tool contract

## Standard Client Behavior

When this repository is imported into an MCP-capable coding tool, the expected behavior is:

1. read project context
2. identify the target capability
3. inspect matching example and docs
4. use stable tool names
5. keep changes aligned with the platform model

## Standard Server Naming

Use a short, descriptive server name:

- `pclikaPlatform`

This name should be reused consistently where client tooling allows custom MCP server naming.

## Standard Server Intent

The future Pclika MCP bridge should act as:

- a hardware capability broker
- a runtime-aware tool surface
- a build and flash coordination layer
- a structured entry point for AI-assisted embedded development

## Standard Configuration Locations

Official client-specific configuration differs by tool.

The repository provides templates under:

- `configs/mcp/codex.config.toml`
- `configs/mcp/claude-code.commands.md`
- `configs/mcp/cursor.mcp.json`
- `configs/mcp/vscode.mcp.json`
- `configs/mcp/opencode.jsonc`

## Tool-Specific Notes

### Codex

Codex supports:

- STDIO MCP servers
- Streamable HTTP MCP servers

Codex stores configuration in `config.toml`, either user-scoped or project-scoped.

### Claude Code

Claude Code supports MCP server management through the `claude mcp` CLI.

This repository currently provides command templates and usage guidance rather than a bundled bridge executable.

### Cursor

Cursor supports MCP and reads MCP configuration from `mcp.json`.

### OpenCode

OpenCode supports both local and remote MCP servers configured under `mcp` in `opencode.json` or `opencode.jsonc`.

### VS Code Agent Mode

VS Code supports MCP servers in Agent flows through `mcp.json`.

## Future Bridge Activation

Once the Pclika bridge is implemented, maintainers should update all templates with:

- final local command
- final remote URL if any
- required environment variables
- authentication behavior
- timeout recommendations

## Related Files

- [mcp-development-flow.md](mcp-development-flow.md)
- [../../bridge/tool-schemas/base-device-tools.json](../../bridge/tool-schemas/base-device-tools.json)
- [../../bridge/server-instructions.md](../../bridge/server-instructions.md)
- [../../configs/mcp/README.md](../../configs/mcp/README.md)
