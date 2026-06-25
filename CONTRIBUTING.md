# Contributing

Thanks for helping build the Pclika MCP Platform.

This repository is intended to become the open-source foundation for Pclika hardware, firmware, MCP tooling, examples, and website experience.

## Before You Start

Read these first:

- [README.md](README.md)
- [AGENTS.md](AGENTS.md)
- [docs/README.md](docs/README.md)
- [docs/architecture/core-platform.md](docs/architecture/core-platform.md)

## Contribution Principles

- Prefer reusable platform building blocks over one-off product code
- Keep hardware, runtime, MCP tools, docs, and examples aligned
- Extend existing capability families before creating new abstractions
- Keep starter workflows simple and easy to understand

## Expected Change Pattern

For most new capabilities, a complete contribution should touch several areas:

1. Hardware definition or module plan
2. Firmware or runtime capability
3. MCP tool schema or bridge behavior
4. Example project
5. Documentation

## Documentation Rules

- Use clear Markdown
- Keep headings short and descriptive
- Prefer platform-oriented wording over vendor-specific marketing language
- Document assumptions and boundaries

## Pull Request Checklist

- Scope is clear
- Docs are updated
- Examples are added or updated when relevant
- New tool names are consistent with existing naming
- New hardware capabilities map cleanly into the platform model

## Large Assets

Avoid committing unnecessary large binary files unless they are essential project assets.

## Licensing Note

See [docs/legal/licensing.md](docs/legal/licensing.md) before contributing material that may need a specific license path.
