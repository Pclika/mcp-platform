# Contributing

Thanks for helping build the Pclika HDL Platform.

This repository is the open-source foundation for Pclika's FPGA and HDL AI development layer —
including RTL IP cores, the MCP bridge, toolchain integration, examples, and documentation.

## Before You Start

Read these first:

- [README.md](README.md)
- [AGENTS.md](AGENTS.md)
- [docs/architecture/platform.md](docs/architecture/platform.md)
- [docs/toolchain/setup.md](docs/toolchain/setup.md)
- [docs/hardware/hardware-list.md](docs/hardware/hardware-list.md)

## Contribution Principles

- RTL must be synthesizable with zero warnings on the target device (iCE40UP5K first)
- Every IP module must have a corresponding testbench in `hdl/tb/`
- Prefer open-toolchain-compatible constructs — no vendor-specific primitives unless justified
- Keep IP modules parameterized; avoid hardcoded frequencies, widths, or pin numbers
- Extend the existing IP library before creating new abstractions

## RTL Contribution Checklist

Before submitting any new or modified Verilog:

- [ ] Module name follows `pclika_<function>` convention
- [ ] All registers have `_r` suffix; all wires have `_w` suffix
- [ ] Synchronous reset used throughout (not async `negedge rst_n`)
- [ ] No `$display`, `#delay`, or `initial` blocks in synthesizable RTL
- [ ] All `case` statements have a `default` branch
- [ ] `localparam` used for all derived constants (no magic numbers)
- [ ] Testbench provided in `hdl/tb/tb_<module>.v`
- [ ] Testbench passes with Verilator: `verilator --lint-only -Wall`
- [ ] SPDX license header added: `CERN-OHL-S-2.0` for RTL, `Apache-2.0` for bridge code

## Expected Change Pattern

A complete IP contribution should touch:

1. `hdl/rtl/pclika_<module>.v` — synthesizable RTL
2. `hdl/tb/tb_pclika_<module>.v` — simulation testbench
3. `docs/hardware/hardware-list.md` — update IP table if new hardware is supported
4. `examples/<name>/` — at least one working example using the new IP
5. `bridge/mcp-server/pclika_hdl_bridge/toolchain.py` — add MCP tool if applicable

## MCP Bridge Contributions

- Tool names must be `snake_case` and descriptive: `synth_run`, `timing_report`
- All tool inputs and outputs must be JSON-serializable structured data — no raw log text
- Add the tool schema to `bridge/tool-schemas/hdl-tools.json`

## Documentation Rules

- Write in clear English (or with Chinese translation for hardware reference content)
- Keep headings short and descriptive
- Prefer tables over prose for hardware specs and pin maps
- Include a `## Seal` line at the bottom of any new doc: `PCK-MMXXVI-9198580D`

## Pull Request Checklist

- [ ] Scope is clearly described in the PR title
- [ ] RTL checklist above is satisfied (for RTL changes)
- [ ] `docs/hardware/hardware-list.md` updated if new hardware supported
- [ ] No binary files committed unless essential (no `.bit`, `.bin` bitstreams)
- [ ] New module names are consistent with `pclika_*` naming convention

## Licensing

- RTL files (`hdl/`) → **CERN-OHL-S v2** — see [HARDWARE_LICENSE.md](HARDWARE_LICENSE.md)
- Bridge and scripts (`bridge/`, `toolchain/`, `configs/`) → **Apache-2.0** — see [LICENSE](LICENSE)
- Documentation (`docs/`, `prompts/`) → **CC BY 4.0** — see [DOCS_LICENSE.md](DOCS_LICENSE.md)

Add the correct SPDX identifier to every new file header.
