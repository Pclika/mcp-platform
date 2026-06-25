# Open-Source Architecture

## Goal

Create an open-source platform that developers can import into GitHub-centric and MCP-compatible coding tools, then extend rapidly with consistent structure and reusable context.

## Repository Strategy

Use a monorepo for the initial platform.

Why:

- shared docs and prompts
- shared examples
- shared tool schemas
- easier onboarding for AI tools
- one stable entry point for GitHub import

## Primary Areas

- `firmware/` device runtime and board support
- `bridge/` MCP bridge and tool contracts
- `examples/` reference projects
- `prompts/` AI prompts and system context
- `docs/` human and AI-readable platform documentation
- `website/` front-end platform and product experience
- `scripts/` helper commands and automation hooks

## Recommended Licensing Model

Software:

- `Apache-2.0` or `MIT`

Hardware design files:

- `CERN-OHL-S` or another hardware-friendly open license

Documentation:

- `CC BY 4.0`

The exact license decision should be made before the public launch.

## Stable Interfaces

The platform should keep four interfaces stable:

- hardware module interfaces
- runtime capability interfaces
- MCP tool schemas
- example project structure

## AI Import Readiness

To make the repository useful when imported into Claude, Codex, Cursor, or OpenCode:

- keep a strong root `README.md`
- include a root `AGENTS.md`
- keep examples small and task-oriented
- define a clear extension pattern
- include prompt templates
- document how tools map to runtime capabilities

## Release Boundaries

### v0 Foundation

- baseboard definition
- runtime skeleton
- MCP bridge contract
- starter examples
- website plan

### v1 Public Open-Source Launch

- stable docs
- base examples
- tool schema set
- first module family support

### v2 Platform Expansion

- more modules
- richer website selection flow
- advanced AI-assisted workflows
