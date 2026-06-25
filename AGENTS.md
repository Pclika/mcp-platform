# Pclika AI Agent Guide

This repository is designed to be imported into Claude, Codex, Cursor, OpenCode, and other MCP-compatible coding tools.

## Mission

Use this repository as the shared foundation for all Pclika MCP Platform hardware and software expansion work.

The goal is to make new feature development predictable:

- Choose a hardware capability
- Reuse the base platform
- Extend the runtime and tool schema
- Add or update examples
- Keep docs and prompts in sync

## Ground Rules

- Treat `docs/architecture/core-platform.md` as the platform source of truth
- Keep the ESP32-S3 base platform stable unless there is a deliberate platform revision
- Prefer extending module interfaces over introducing one-off custom flows
- Every new hardware feature should map to:
  - a runtime capability
  - an MCP tool or tool extension
  - at least one example
  - documentation updates
- Keep the developer experience simple enough that a user can start from guided options

## Read First

- `README.md`
- `docs/software/standard-invocation.md`
- `docs/architecture/core-platform.md`
- `docs/hardware/base-platform.md`
- `docs/hardware/extension-modules.md`
- `docs/software/open-source-architecture.md`
- `docs/software/mcp-development-flow.md`

## Standard Invocation Rule

When this repository is used in Claude, Codex, Cursor, OpenCode, or similar MCP-compatible tools:

- use the repository entry order defined in `docs/software/standard-invocation.md`
- prefer the standard server name `pclikaPlatform`
- prefer stable tool names over ad hoc new names

## Core Extension Pattern

When adding a new capability such as vision, servo control, or a sensor family:

1. Define the hardware interface
2. Define the firmware runtime module
3. Define the MCP tool surface
4. Add an example workflow
5. Update prompts and docs

## Naming

- Hardware modules: `mod_<family>_<name>`
- Runtime components: `runtime_<capability>`
- MCP tools: verb-oriented names such as `sensor_read`, `servo_move`, `camera_capture`
- Examples: short workflow names such as `hello-mcp`, `env-monitor`, `servo-control`

## Priority

Favor reusable building blocks over feature-specific code.
