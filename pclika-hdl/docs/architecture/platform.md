# Pclika HDL Platform Architecture

## Purpose

Pclika HDL Platform is the open-source HDL AI development foundation for FPGA and programmable logic.

It provides:

- A standard HDL project structure and toolchain integration
- An MCP bridge layer that exposes synthesis, timing, and simulation state to AI tools
- Device-specific constraint templates and resource profiles
- Reusable IP blocks and working examples
- Prompts and docs that make AI-assisted RTL development immediately productive

---

## Platform Layers

### Layer 1 — HDL Source Layer

This layer contains all synthesizable RTL and testbenches.

Structure:

```
hdl/
  rtl/           ← synthesizable Verilog / SystemVerilog / VHDL
  tb/            ← simulation testbenches
  constraints/   ← device-specific pin and timing constraints
  ip/            ← reusable IP blocks (UART, SPI, I2C, PWM, FIFO...)
```

Rules:

- All RTL in `hdl/rtl/` must be synthesizable
- Simulation-only code goes only in `hdl/tb/`
- Avoid `initial` blocks outside testbenches
- Parameters are preferred over hardcoded constants

---

### Layer 2 — Toolchain Layer

This layer defines the build scripts and toolchain invocation.

Primary open-source stack:

| Tool | Role | Target |
|------|------|--------|
| Yosys | Synthesis | iCE40, ECP5, (Zynq partial) |
| nextpnr | Place & Route | iCE40, ECP5 |
| iceprog | Bitstream Flash | iCE40 |
| openFPGALoader | Bitstream Flash | ECP5, multi-device |
| Verilator | Simulation | All (fast cycle-accurate) |
| GHDL | VHDL Simulation | All (VHDL-first flows) |
| GTKWave | Waveform View | VCD / FST viewing |

Build script location: `toolchain/scripts/`

Standard build commands:

```bash
make lint          # Verilator lint check
make synth         # Yosys synthesis
make impl          # nextpnr P&R
make timing        # timing report extraction
make flash         # bitstream to device
make sim TARGET=tb_<name>   # run testbench simulation
```

---

### Layer 3 — MCP Bridge Layer

This is what makes Pclika HDL an AI-native platform.

The MCP bridge (`pclika-hdl-bridge`) is a Python STDIO MCP server that:

- Wraps toolchain invocations
- Parses synthesis and timing reports
- Returns structured JSON to AI tools
- Manages long-running jobs (synthesis can take seconds to minutes)

Standard tool set:

```
device_info         → device name, package, speed grade, resource totals
synth_run           → trigger Yosys synthesis, return job ID
synth_status        → synthesis result, module count, warnings, errors
impl_run            → trigger nextpnr P&R, return job ID
impl_status         → routing success/fail, wire usage
timing_report       → critical path delay, worst slack, violated paths
resource_usage      → LUT / FF / BRAM / DSP (used / total / %)
sim_run             → trigger Verilator/GHDL simulation
sim_result          → pass/fail, assertion results, log tail (last 50 lines)
lint_report         → all lint warnings and errors by file/line
constraint_validate → .pcf / .lpf / .xdc validation result
bitstream_flash     → flash bitstream, return programming result
waveform_export     → export VCD/FST for last N simulation cycles
```

Bridge location: `bridge/mcp-server/`  
Tool schemas: `bridge/tool-schemas/`

---

### Layer 4 — Experience Layer

This layer is what users and AI tools interact with first.

It includes:

- `examples/` — complete working projects per device and use case
- `prompts/` — AI-ready prompt templates for synthesis debug, timing closure, RTL review
- `docs/` — human and AI-readable platform and device documentation
- `configs/mcp/` — client configuration templates for Claude, Codex, Cursor, VS Code

---

## Device Profile System

Each supported device has a profile document at `docs/devices/<device>.md` that defines:

- Device name, family, package options
- Available resources (LUT / FF / BRAM / DSP / PLL)
- Clock constraints baseline
- Constraint file format (.pcf / .lpf / .xdc)
- Toolchain invocation specifics
- Flash method and interface
- Known limitations or caveats

### First Device: iCE40UP5K

```
Device:    iCE40UP5K
Package:   SG48 (48-pin QFN) or UWG30
LUT4:      5280
FF:        5280
BRAM:      120 Kbit (30 × 4 Kbit blocks)
DSP:       8 × 16-bit multiplier
PLL:       1
Toolchain: Yosys + nextpnr-ice40 + iceprog
Constraint: .pcf format
```

---

## MCP Development Flow

```
1. Import repository into AI coding tool
2. Connect FPGA board via USB
3. Start pclika-hdl-bridge
4. AI tool reads AGENTS.md and platform docs
5. User describes desired behavior
6. AI tool writes or modifies RTL
7. AI tool calls lint_report → synth_run → timing_report
8. Iterate until timing clean and resources within budget
9. AI tool calls bitstream_flash
10. Verify on hardware
```

---

## Extension Pattern

Every new capability follows this path:

1. Define module interface (ports, parameters)
2. Write RTL in `hdl/rtl/`
3. Write testbench in `hdl/tb/`
4. Add device constraint entry in `hdl/constraints/<device>/`
5. Create working example in `examples/`
6. Add MCP tool if new hardware interaction is needed
7. Update docs and prompts

This pattern keeps the platform consistent as it scales.

---

## Relationship to Pclika MCP Platform

Pclika HDL is a sibling project under the Pclika organization.

| | Pclika MCP Platform | Pclika HDL Platform |
|-|---------------------|---------------------|
| Target | MCU / Embedded | FPGA / Programmable Logic |
| Primary MCU | ESP32-S3, STM32 | iCE40, ECP5, Zynq |
| Language | C / C++ | Verilog / VHDL / SystemVerilog |
| Bridge | pclika-bridge (Python) | pclika-hdl-bridge (Python) |
| MCP server | pclikaPlatform | pclikaHDL |
| Phase | Phase 1 active | Phase 1 planning |

Both projects share:

- Brand identity and Origin Codex
- MCP bridge protocol design principles
- Documentation structure
- Tool naming conventions
- Community and contribution standards
