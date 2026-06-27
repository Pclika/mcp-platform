<?xml version="1.0" encoding="utf-8"?>
<!--
  Pclika HDL Platform — Official Repository
  PCK-MMXXVI-9198580D
  Verify: https://pclika.com/verify/PCK-9198580D
-->

# Pclika HDL Platform

> **Pclika HDL** is the open-source HDL AI development platform under the Pclika ecosystem.  
> It is not a synthesis tool. It is the interface between human intent and silicon logic.

Pclika HDL brings MCP-native AI development to FPGA and programmable logic — the same way Pclika MCP Platform does for MCU/embedded development.

Import this repository into Claude, Codex, Cursor, or OpenCode, and your AI tool immediately understands: your target device, the active toolchain, current design state, synthesis results, timing constraints, and how to run simulations.

---

## Project Description

FPGA development has always carried an unusually high entry cost — not because logic design is inherently complex, but because the toolchain is. Vendor GUIs, proprietary constraint formats, opaque synthesis logs, and device-specific primitive libraries create a barrier that even experienced engineers spend weeks crossing before writing a single useful line of RTL.

Pclika HDL is built on a different premise: **the toolchain should be transparent, and the AI should be the interface.**

Rather than wrapping FPGA development inside another GUI, Pclika HDL exposes the entire open-source toolchain — Yosys for synthesis, nextpnr for place-and-route, Verilator for simulation — through a structured MCP bridge. This means an AI coding tool connected to `pclika-hdl-bridge` doesn't just generate Verilog: it can synthesize it, read the resource report, identify the critical path, fix the timing violation, and flash the bitstream — all without leaving the conversation.

The first target is the **iCE40UP5K**, chosen deliberately. It has 5280 LUT4s, eight DSPs, a single PLL, and a fully open toolchain with no license dependency. It is small enough to understand completely, and real enough to build production-quality peripheral controllers on. The IP library starts here — UART, SPI, PWM — and grows outward toward ECP5 and eventually Zynq-class SoCs.

The design philosophy follows three rules. First, every RTL module must be synthesizable with zero warnings on the first try — no simulation-only constructs, no magic numbers, no undeclared ports. Second, every example must produce a working bitstream, not just compile. Third, every MCP tool must return structured, parseable output — not log text — so AI tools can act on results rather than interpret them.

This is not a synthesis framework. It is a **context layer**: the missing piece that lets AI tools reason about FPGA state the same way they reason about software — with full visibility into what the hardware is, what it is doing, and what it needs next.

---

## What This Is

A unified HDL AI development layer built around four ideas:

- **Open toolchain first** — Yosys + nextpnr + Verilator, fully open source
- **MCP-ready** — AI tools call `synth_run`, `timing_report`, `resource_usage` against real FPGA state
- **Device-agnostic** — iCE40 first, then ECP5, then Zynq-class devices
- **Language-inclusive** — Verilog, SystemVerilog, VHDL all supported

---

## Core Layers

```
HDL Source Layer        →  Verilog / SystemVerilog / VHDL
Toolchain Layer         →  Yosys / nextpnr / Verilator / GHDL
MCP Bridge Layer        →  synth_run / timing_report / resource_usage / sim_run
Experience Layer        →  examples / prompts / docs / constraint templates
```

---

## Supported Hardware

| Board | FPGA | Toolchain | Status |
|-------|------|-----------|--------|
| iCEBreaker v1.0 ★ | iCE40UP5K SG48 | Yosys + nextpnr + iceprog | Phase 1 — primary |
| Upduino v3.1 | iCE40UP5K SG48 | Yosys + nextpnr + iceprog | Phase 1 — compatible |
| iCE40-HX8K Breakout | iCE40HX8K CT256 | Yosys + nextpnr + iceprog | Phase 1 — partial |
| ColorLight i5 | Lattice ECP5 LFE5U-25F | Yosys + nextpnr + openFPGALoader | Phase 2 |
| Arty A7-35T | AMD Artix-7 XC7A35T | Yosys + Vivado (impl) | Phase 3 |

**IP Library × Hardware mapping** → [`docs/hardware/hardware-list.md`](docs/hardware/hardware-list.md)

| Interface | IP Module | Supported Hardware |
|-----------|-----------|-------------------|
| UART 8N1 | `pclika_uart_rx` + `pclika_uart_tx` | iCEBreaker (FT2232H), Upduino (CH340), any PMOD-UART |
| SPI Master (Mode 0) | `pclika_spi_master` | W25Q128 Flash, BME280, MAX31865, SSD1309, ST7789 |
| PWM / Servo 50 Hz | `pclika_pwm` | SG90 / MG996R servos, ESC, RGB LED dimming |
| I2C Master | `pclika_i2c_master` | MPU6050, BMP280, SSD1306 I2C — *Phase 2* |
| WS2812B | `pclika_ws2812` | Addressable RGB LED strips — *Phase 2* |

---

## MCP Tool Set

```
device_info         → FPGA target, package, speed grade, available resources
synth_run           → trigger Yosys synthesis
synth_status        → latest synthesis result, warnings, errors
impl_run            → trigger nextpnr place & route
impl_status         → P&R result, routing success/fail
timing_report       → critical path, worst slack, clock domains
resource_usage      → LUT / FF / BRAM / DSP utilization (used / total / %)
sim_run             → trigger Verilator/GHDL simulation
sim_result          → pass/fail, assertion count, log tail
lint_report         → HDL lint warnings and errors
constraint_validate → validate .pcf / .lpf / .xdc file
bitstream_flash     → flash bitstream to connected FPGA via USB
waveform_export     → export VCD/FST waveform snippet (last N cycles)
```

---

## Repository Layout

```
docs/
  architecture/
  toolchain/
  devices/
  software/
hdl/
  rtl/              ← synthesizable RTL source
  tb/               ← testbenches
  constraints/      ← .pcf / .lpf / .xdc per device
  ip/               ← reusable IP blocks
bridge/
  mcp-server/       ← Python MCP bridge for HDL toolchain
  tool-schemas/     ← JSON Schema for all MCP tools
toolchain/
  scripts/          ← build / synth / impl / sim scripts
  docker/           ← Docker image with full open toolchain
examples/
  blink/            ← LED blink on iCE40UP5K
  uart-echo/        ← UART loopback example
  i2c-controller/   ← I2C master implementation
  spi-bridge/       ← SPI to UART bridge
  pwm-gen/          ← Configurable PWM generator
prompts/
  common/
  synth-workflow.md
  timing-debug.md
  rtl-review.md
configs/
  mcp/
    claude-code.commands.md
    cursor.mcp.json
    codex.config.toml
    vscode.mcp.json
```

---

## Start Here

1. `README.md` — this file
2. `AGENTS.md` — AI tool guide
3. `docs/architecture/platform.md` — platform architecture
4. `docs/toolchain/setup.md` — toolchain installation
5. `examples/blink/` — first working example

---

## Quick Start

```bash
# Install bridge
pip install pclika-hdl-bridge

# Connect your iCE40 board via USB
pclika-hdl-bridge --device ice40up5k --port /dev/ttyUSB0

# In Claude / Codex / Cursor — the following tools are now available:
# device_info / synth_run / timing_report / resource_usage / bitstream_flash
```

---

## Repository Status

Early-stage open-source foundation. Current focus:

- toolchain integration architecture
- MCP bridge contract definition
- iCE40UP5K first device support
- example project structure

---

## License

This repository uses different licenses for different components:

| Component | License | Files |
|-----------|---------|-------|
| Software (bridge, scripts, configs) | [Apache-2.0](LICENSE) | `bridge/`, `toolchain/`, `configs/` |
| RTL hardware designs | [CERN-OHL-S v2](HARDWARE_LICENSE.md) | `hdl/`, `examples/*/rtl/`, `examples/*/constraints/` |
| Documentation & prompts | [CC BY 4.0](DOCS_LICENSE.md) | `docs/`, `prompts/`, `*.md` |

**In short:**
- Use and modify the bridge software freely in commercial projects (Apache-2.0).
- Use the RTL IP cores freely, but if you distribute products incorporating them, you must open-source the modified RTL (CERN-OHL-S v2).
- Reproduce or adapt the documentation with attribution (CC BY 4.0).

Third-party toolchain components (Yosys, nextpnr, Verilator, iceprog) retain their own licenses — see [`NOTICE`](NOTICE).

Copyright 2026 [Pclika](https://pclika.com)

---

## Community

- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Contributing](CONTRIBUTING.md)
- [Security](SECURITY.md)

---

## Part of the Pclika Ecosystem

| Repository | Role |
|------------|------|
| [Pclika/mcp-platform](https://github.com/Pclika/mcp-platform) | MCU / Embedded MCP Platform (ESP32, STM32) |
| [Pclika/pclika-hdl](https://github.com/Pclika/pclika-hdl) | FPGA / HDL AI Development Platform |
