# Pclika MCP Platform

Pclika MCP Platform is the open-source foundation for the Pclika MCP Ready Kit ecosystem.

It defines a standard hardware baseboard, a reusable embedded runtime, a bridge layer for MCP-compatible AI tools, and a website-driven configuration experience that helps users start embedded development with a few guided choices.

## Vision

Pclika is not a single board product. It is a standard platform that helps developers, makers, educators, and product teams go from idea to prototype faster with AI-native embedded workflows.

The platform is built around four ideas:

- Standard hardware first
- Open-source software foundation
- MCP-ready AI development workflow
- Modular expansion for sensing, vision, motion, and control

## Repository Status

This repository is an early-stage open-source foundation for the Pclika MCP ecosystem.

The current focus is on:

- platform architecture
- hardware and expansion standards
- MCP workflow definition
- documentation structure
- repository governance basics

## Core Layers

- Hardware Platform: ESP32-S3 baseboard and expansion standard
- Device Runtime: reusable firmware architecture and drivers
- MCP Bridge: tool layer for Claude, Codex, Cursor, OpenCode, and similar tools
- Experience Layer: examples, prompts, docs, templates, and website flows

## Product Line

- `KIT-MCP-01` Pclika MCP Ready Kit Basic
- `KIT-MCP-02` Pclika MCP Ready Kit Pro
- `KIT-MCP-03` Pclika MCP Ready Kit Industrial
- `KIT-MCP-04` Pclika MCP Ready Kit Edu Lab

## Expansion Families

- Vision
- Sensor
- Motion and Servo
- Display and HMI
- Industrial IO
- Power and Battery

## Repository Layout

```text
docs/
  architecture/
  hardware/
  software/
  website/
firmware/
  esp-idf/
  arduino/
bridge/
  mcp-server/
  tool-schemas/
examples/
prompts/
scripts/
website/
```

## Start Here

- Public docs index: [docs/README.md](docs/README.md)
- Platform overview: [docs/architecture/core-platform.md](docs/architecture/core-platform.md)
- Project foundation: [docs/architecture/project-foundation.md](docs/architecture/project-foundation.md)
- Hardware definition: [docs/hardware/base-platform.md](docs/hardware/base-platform.md)
- Platform families and tiers: [docs/architecture/platform-families-and-tiers.md](docs/architecture/platform-families-and-tiers.md)
- Expansion plan: [docs/hardware/extension-modules.md](docs/hardware/extension-modules.md)
- Memory, interface, wireless, and security standard: [docs/hardware/memory-interface-security.md](docs/hardware/memory-interface-security.md)
- Open-source architecture: [docs/software/open-source-architecture.md](docs/software/open-source-architecture.md)
- MCP workflow: [docs/software/mcp-development-flow.md](docs/software/mcp-development-flow.md)
- Standard invocation guide: [docs/software/standard-invocation.md](docs/software/standard-invocation.md)
- Website plan: [docs/website/website-platform.md](docs/website/website-platform.md)
- Product specification: [docs/product/mcp-ready-kit-spec.md](docs/product/mcp-ready-kit-spec.md)

## Standard Invocation

This repository includes a standard way to connect MCP-compatible coding tools to the platform.

- Standard invocation guide: [docs/software/standard-invocation.md](docs/software/standard-invocation.md)
- MCP config templates: [configs/mcp/README.md](configs/mcp/README.md)
- Server instruction draft: [bridge/server-instructions.md](bridge/server-instructions.md)

## GitHub Readiness

Repository-level files prepared for publication:

- [CONTRIBUTING.md](CONTRIBUTING.md)
- [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)
- [SECURITY.md](SECURITY.md)
- [SUPPORT.md](SUPPORT.md)
- [docs/legal/licensing.md](docs/legal/licensing.md)

Repository-level legal and brand files:

- [LICENSE](LICENSE)
- [HARDWARE_LICENSE.md](HARDWARE_LICENSE.md)
- [DOCS_LICENSE.md](DOCS_LICENSE.md)
- [TRADEMARKS.md](TRADEMARKS.md)
- [NOTICE](NOTICE)

## Current Scope

The first milestone is to define and stabilize the core platform:

- Baseboard capabilities
- Expansion module standard
- MCP bridge contract
- Example flows for AI-assisted embedded development
- Website information architecture and kit configuration flow

## Long-Term Goal

The long-term goal is for a developer to import this repository into an MCP-compatible coding tool, choose a target hardware capability, and immediately unlock a working development path for firmware, hardware control, examples, and documentation.
