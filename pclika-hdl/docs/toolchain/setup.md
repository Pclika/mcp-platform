# Toolchain Setup — Pclika HDL Platform

## Overview

Pclika HDL uses a fully open-source toolchain for iCE40 and ECP5 targets:

| Tool | Role | Install |
|------|------|---------|
| Yosys | RTL synthesis | `apt install yosys` |
| nextpnr-ice40 | Place & route (iCE40) | `apt install nextpnr-ice40` |
| fpga-icestorm | icepack + iceprog | `apt install fpga-icestorm` |
| Verilator | Lint + simulation | `apt install verilator` |
| GTKWave | Waveform viewer | `apt install gtkwave` |

---

## Option A — OSS CAD Suite (Recommended)

The easiest method: one download, all tools at latest versions.

```bash
# Download from https://github.com/YosysHQ/oss-cad-suite-build/releases
# Choose your platform (linux-x64, darwin-arm64, windows-x64)

# Linux example:
wget https://github.com/YosysHQ/oss-cad-suite-build/releases/latest/download/oss-cad-suite-linux-x64-<date>.tgz
tar xf oss-cad-suite-linux-x64-<date>.tgz
source oss-cad-suite/environment

# Verify
yosys --version
nextpnr-ice40 --version
iceprog --version
verilator --version
```

---

## Option B — Docker (no local install)

```bash
# Build the Pclika HDL Docker image (includes all tools)
cd toolchain/docker
docker build -t pclika-hdl .

# Run a synthesis from your project directory
docker run --rm -v $(pwd):/work pclika-hdl \
    make -C /work synth TOP=blink_top

# Flash requires USB passthrough (Linux only via --device)
docker run --rm --privileged -v $(pwd):/work pclika-hdl \
    make -C /work flash
```

---

## Option C — Package manager (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
    yosys \
    nextpnr-ice40 \
    fpga-icestorm \
    verilator \
    gtkwave

# Verify versions
yosys --version       # should be ≥ 0.36
nextpnr-ice40 --version
verilator --version   # should be ≥ 5.0 for --binary support
```

---

## Install pclika-hdl-bridge

```bash
pip install pclika-hdl-bridge

# Verify
pclika-hdl-bridge --help
```

---

## USB Permissions (Linux)

iCEBreaker and most FPGA programmers use FTDI or similar USB chips.

```bash
# Add udev rule for FTDI
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="0403", MODE="0666"' \
    | sudo tee /etc/udev/rules.d/99-pclika-hdl.rules

# Also iCEBreaker (Lattice)
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="0403", ATTR{idProduct}=="6010", MODE="0666"' \
    >> /etc/udev/rules.d/99-pclika-hdl.rules

sudo udevadm control --reload-rules
sudo udevadm trigger

# Add yourself to plugdev group
sudo usermod -aG plugdev $USER
# Log out and back in for group change to take effect
```

---

## Verify Full Flow

Run this from `examples/blink/` to confirm the complete toolchain works:

```bash
# 1. Lint
make lint

# 2. Synthesize
make synth TOP=blink_top

# 3. Place & route
make impl TOP=blink_top FREQ=12

# 4. Check timing
make timing

# 5. Simulation
make sim TARGET=tb_blink_top

# 6. Flash (connect iCEBreaker first)
make flash
```

All steps should complete without errors. Expected timing output:
```
Max frequency for clock 'clk': 87.xx MHz (constraint: 12 MHz)
Worst slack: +xx.xx ns
```

---

## MCP Bridge — Quick Start

```bash
# From your project root (where hdl/ lives)
pclika-hdl-bridge --device ice40up5k --freq 12

# Debug mode (shows subprocess output)
pclika-hdl-bridge --device ice40up5k --debug

# Specify project root explicitly
pclika-hdl-bridge --project /path/to/project --device ice40up5k
```

Add the MCP config from `configs/mcp/cursor.mcp.json` to your AI tool, then call `device_info` to verify the connection.

---

## Troubleshooting

**`yosys: command not found`**
→ Install via OSS CAD Suite or `apt install yosys`

**`iceprog: unable to find iCEBreaker`**
→ Check USB connection and udev rules above

**`nextpnr-ice40: routing failed`**
→ Try a different seed: `make impl SEED=42`

**Verilator `--binary` not found**
→ You need Verilator ≥ 5.0. Install via OSS CAD Suite.
