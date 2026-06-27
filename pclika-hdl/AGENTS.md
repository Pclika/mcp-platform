# Pclika HDL — AI Agent Guide

This repository is designed to be imported into Claude, Codex, Cursor, OpenCode, and other MCP-compatible coding tools.

## Mission

Use this repository as the shared foundation for all Pclika HDL Platform development work.

The goal is to make FPGA and HDL development predictable with AI tools:

- Choose a target device
- Write or modify RTL
- Run synthesis and check results via MCP tools
- Iterate on timing and resource usage
- Flash the bitstream when ready

## Ground Rules

- Treat `docs/architecture/platform.md` as the HDL platform source of truth
- Keep the iCE40UP5K as the primary first-generation target
- Prefer synthesizable, portable RTL over vendor-specific primitives
- Every new IP block or example should have:
  - RTL source in `hdl/rtl/` or `hdl/ip/`
  - Testbench in `hdl/tb/`
  - Constraint file in `hdl/constraints/<device>/`
  - Example project in `examples/`
  - Documentation update

## Read First

1. `README.md`
2. `docs/architecture/platform.md`
3. `docs/toolchain/setup.md`
4. `docs/devices/ice40up5k.md`
5. The closest example under `examples/`

## Standard MCP Tool Usage

When this repository is connected via `pclika-hdl-bridge`:

- Use `device_info` before any synthesis or flash operation
- Use `lint_report` before `synth_run` — catch errors early
- Use `synth_status` after `synth_run` completes — check for warnings
- Use `timing_report` after `impl_run` — verify all clocks meet timing
- Use `resource_usage` to verify utilization fits the target device
- Use `bitstream_flash` only after timing is clean

## Standard Server Name

Use `pclikaHDL` as the MCP server name in all client configurations.

## RTL Naming Conventions

- Modules: `pclika_<function>` (e.g., `pclika_uart_rx`, `pclika_spi_master`)
- Testbenches: `tb_<module_name>` (e.g., `tb_pclika_uart_rx`)
- Parameters: `UPPER_SNAKE_CASE`
- Ports: `lower_snake_case`
- Internal signals: `lower_snake_case` with `_r` suffix for registers, `_w` for wires

## Core Extension Pattern

When adding a new IP block or capability:

1. Define the module interface and parameters
2. Write synthesizable RTL in `hdl/rtl/`
3. Write a testbench in `hdl/tb/`
4. Add device constraints in `hdl/constraints/<device>/`
5. Create a working example in `examples/<name>/`
6. Update `docs/` with module description and port list
7. Verify with `lint_report` → `synth_run` → `impl_run` → `timing_report`

## Toolchain Order

For a clean build cycle:

```
lint_report       ← fix all lint errors first
synth_run         ← synthesize with Yosys
synth_status      ← check for unresolved modules or warnings
impl_run          ← place & route with nextpnr
impl_status       ← check for routing failures
timing_report     ← verify timing closure
resource_usage    ← check utilization fits device
bitstream_flash   ← flash if all above pass
```

## Priority

Favor portable, vendor-agnostic RTL. Avoid unnecessary use of vendor primitives unless there is a clear performance or resource reason.
