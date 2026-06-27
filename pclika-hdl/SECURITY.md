# Security Policy

## Scope

This repository includes:

- RTL IP cores (Verilog — synthesizable hardware)
- MCP bridge server (Python — runs on host machine)
- Toolchain scripts and Dockerfile
- Configuration templates for AI coding tools

Security issues may affect:

- The MCP bridge process and its exposed tool surface
- Toolchain script execution (synth, impl, flash commands)
- Docker image integrity
- Bitstream authenticity (flash target verification)
- AI tool configuration files (command injection via config)

## Reporting a Vulnerability

Please **do not open a public GitHub issue** for security vulnerabilities.

Report security concerns privately to:

**starinvc@gmail.com**

Subject line: `[pclika-hdl] Security: <brief description>`

We aim to acknowledge reports within 72 hours.

## What to Include

- Affected component (`bridge/`, `toolchain/`, `configs/`, RTL)
- Affected version or commit hash
- Reproduction steps
- Expected impact
- Whether the issue involves code execution, file access, or network exposure

## Security Priorities

The highest-priority classes for this project are:

- **Command injection** via MCP tool arguments passed to toolchain processes
- **Path traversal** in project root handling (`--project` flag of `pclika-hdl-bridge`)
- **Docker escape** or privilege escalation via USB device passthrough (`--privileged`)
- **Bitstream tampering** — unsigned flash writes to connected FPGA hardware
- **Malicious MCP config** — crafted `cursor.mcp.json` or `vscode.mcp.json` executing arbitrary commands

## Out of Scope

- Bugs in upstream tools (Yosys, nextpnr, Verilator, iceprog) — report to their respective maintainers
- Physical attacks on FPGA hardware
- Side-channel attacks on synthesized RTL designs

## Supported Versions

| Component | Supported |
|-----------|-----------|
| `pclika-hdl-bridge` latest on `main` | ✅ |
| Older tagged releases | ⚠️ Best effort |
