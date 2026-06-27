# Device Profile: iCE40UP5K

## Overview

The iCE40UP5K is the first-class target device for Pclika HDL Platform.

It was chosen because:

- Fully open-source toolchain (Yosys + nextpnr + iceprog)
- No vendor lock-in
- Well-documented with extensive community resources
- AI code coverage is excellent (most open-source FPGA examples use iCE40)
- Affordable and widely available
- Strong fit for learning, prototyping, and MCP-workflow development

---

## Device Specifications

| Parameter | Value |
|-----------|-------|
| Family | Lattice iCE40 UltraPlus |
| Device | iCE40UP5K |
| Process | 40nm |
| LUT4 | 5,280 |
| Flip-Flops | 5,280 |
| BRAM | 120 Kbit (30 × 4 Kbit EBR) |
| SPRAM | 256 Kbit (4 × 64 Kbit) |
| DSP (16×16 mul) | 8 |
| PLL | 1 |
| I/O Pins (SG48) | 39 |
| Operating Voltage | 1.2V core / 1.8V or 3.3V I/O |
| Max System Clock | ~100 MHz (speed grade dependent) |

---

## Package Options

| Package | Pins | Notes |
|---------|------|-------|
| SG48 | 48-pin QFN | Most common, hand-solderable |
| UWG30 | 30-pin Wafer-Level | Very small, harder to hand-solder |

Pclika HDL boards use **SG48** as the standard package.

---

## Toolchain

### Installation

```bash
# Install Yosys (synthesis)
sudo apt install yosys

# Install nextpnr-ice40 (place & route)
sudo apt install nextpnr-ice40

# Install iceprog (bitstream flash)
sudo apt install fpga-icestorm

# Or install all via OSS CAD Suite (recommended)
# https://github.com/YosysHQ/oss-cad-suite-build
```

### Synthesis Command

```bash
yosys -p "synth_ice40 -top <top_module> -json output.json" <source_files>
```

### Place & Route Command

```bash
nextpnr-ice40 --up5k --package sg48 \
  --json output.json \
  --pcf constraints/ice40up5k.pcf \
  --asc output.asc \
  --freq <target_mhz>
```

### Bitstream Pack & Flash

```bash
icepack output.asc output.bin
iceprog output.bin
```

---

## Constraint File Format

iCE40 uses `.pcf` (Physical Constraint Format):

```pcf
# Clock
set_io clk 35

# LEDs
set_io led_r 41
set_io led_g 40
set_io led_b 39

# UART
set_io uart_tx 43
set_io uart_rx 38

# SPI
set_io spi_sck  47
set_io spi_mosi 44
set_io spi_miso 45
set_io spi_cs   46
```

Default constraint template: `hdl/constraints/ice40up5k/default.pcf`

---

## Common Development Boards

Pclika HDL is compatible with:

| Board | Flash | Clock | PSRAM | Notes |
|-------|-------|-------|-------|-------|
| iCEBreaker | 4MB | 12 MHz | No | Most popular iCE40UP5K board |
| Fomu | 1MB | 48 MHz | No | Tiny USB key format |
| iCE40-HX8K Breakout | — | 12 MHz | No | Larger HX8K variant |

---

## Resource Budget Guidelines

For typical Pclika HDL projects:

| Use Case | LUT4 Budget | Notes |
|----------|-------------|-------|
| UART RX/TX (115200 baud) | ~50 | Very lightweight |
| SPI Master (full duplex) | ~100 | |
| I2C Master | ~150 | |
| PWM Generator (8-channel) | ~80 | |
| Simple state machine | ~200-500 | Depends on state count |
| Small RISC-V softcore | ~3000-4500 | Uses most of the device |

Rule: keep each IP block under 20% of total LUTs unless it's the primary design function.

---

## Known Limitations

- No hard multipliers beyond 8 × 16-bit DSP blocks
- SPRAM (256 Kbit) is accessible only as four independent 64 Kbit blocks
- Single PLL limits mixed-frequency designs
- nextpnr routing can fail on very dense designs — try changing the seed
- No partial reconfiguration support in open-source flow
