# Dockerfile — pclika-hdl toolchain environment
#
# Base: OSS CAD Suite (nightly) — includes Yosys, nextpnr-ice40, nextpnr-ecp5,
#       iceprog, Verilator, iverilog, gtkwave
#
# Usage:
#   docker build -t pclika-hdl .
#   docker run --rm -v $(pwd):/project pclika-hdl make -C /project flash
#
# For USB flash access add:
#   --device /dev/bus/usb --privileged
#
# Seal: PCK-MMXXVI-9198580D

FROM ubuntu:22.04 AS base

ENV DEBIAN_FRONTEND=noninteractive
ENV OSS_CAD_VERSION=2024-06-01

# System deps
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl ca-certificates tar xz-utils \
    python3 python3-pip python3-venv \
    libusb-1.0-0 udev \
    make git \
    && rm -rf /var/lib/apt/lists/*

# Install OSS CAD Suite
RUN curl -fsSL \
    "https://github.com/YosysHQ/oss-cad-suite-build/releases/download/${OSS_CAD_VERSION}/oss-cad-suite-linux-x64-${OSS_CAD_VERSION//-/}.tgz" \
    -o /tmp/oss-cad-suite.tgz && \
    tar -xzf /tmp/oss-cad-suite.tgz -C /opt && \
    rm /tmp/oss-cad-suite.tgz

ENV PATH="/opt/oss-cad-suite/bin:${PATH}"

# Verify core tools
RUN yosys --version && \
    nextpnr-ice40 --version && \
    verilator --version

# ── pclika-hdl-bridge ─────────────────────────────────────────────────────
COPY bridge/mcp-server /opt/pclika-hdl/bridge/mcp-server
WORKDIR /opt/pclika-hdl/bridge/mcp-server

RUN python3 -m pip install --no-cache-dir --break-system-packages -e .

# Verify bridge is installed
RUN pclika-hdl-bridge --help

# ── udev rules (for USB flash, copied for reference) ─────────────────────
COPY toolchain/scripts/99-pclika-usb.rules /etc/udev/rules.d/

# ── Working directory ─────────────────────────────────────────────────────
WORKDIR /project

ENTRYPOINT ["/bin/bash", "-lc"]
CMD ["bash"]
