"""
__main__.py — CLI entry point for pclika-hdl-bridge

Usage:
    pclika-hdl-bridge [options]

Options:
    --project PATH     Path to project root (default: current directory)
    --device NAME      Target device: ice40up5k | ecp5 (default: ice40up5k)
    --package PKG      Device package: sg48 | uwg30 (default: sg48)
    --freq MHZ         Target clock frequency in MHz (default: 12)
    --debug            Enable debug logging
"""

import sys
import argparse
import logging
from pathlib import Path

from .toolchain import ToolchainRunner
from .server import HDLMCPServer
from . import __version__, SEAL


def main():
    parser = argparse.ArgumentParser(
        prog="pclika-hdl-bridge",
        description="MCP bridge for Pclika HDL Platform (FPGA AI development)",
    )
    parser.add_argument(
        "--project", default=".",
        help="Path to project root (where hdl/ and toolchain/ live). Default: current dir.",
    )
    parser.add_argument(
        "--device", default="ice40up5k",
        choices=["ice40up5k", "ecp5", "zynq"],
        help="Target FPGA device (default: ice40up5k)",
    )
    parser.add_argument(
        "--package", default="sg48",
        help="Device package (default: sg48)",
    )
    parser.add_argument(
        "--freq", type=float, default=12.0,
        help="Target clock frequency in MHz (default: 12)",
    )
    parser.add_argument(
        "--debug", action="store_true",
        help="Enable debug logging",
    )
    args = parser.parse_args()

    # ── Logging ───────────────────────────────────────────────────────────
    level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(
        stream=sys.stderr,
        level=level,
        format="%(levelname)s pclika.hdl: %(message)s",
    )

    log = logging.getLogger("pclika_hdl.main")
    log.info("pclika-hdl-bridge v%s", __version__)
    log.info("Seal: %s", SEAL)

    # ── Project root ──────────────────────────────────────────────────────
    project_root = Path(args.project).resolve()
    if not project_root.exists():
        log.error("Project root not found: %s", project_root)
        sys.exit(1)

    hdl_dir = project_root / "hdl"
    if not hdl_dir.exists():
        log.warning("hdl/ directory not found at %s", project_root)
        log.warning("Expected structure: <project>/hdl/rtl/, hdl/tb/, hdl/constraints/")

    log.info("Project:  %s", project_root)
    log.info("Device:   %s / %s @ %.1f MHz", args.device, args.package, args.freq)

    # ── Runner + Server ───────────────────────────────────────────────────
    runner = ToolchainRunner(
        project_root=project_root,
        device=args.device,
        package=args.package,
        freq_mhz=args.freq,
    )
    server = HDLMCPServer(runner)

    try:
        server.run()
    except KeyboardInterrupt:
        log.info("Shutting down")
        sys.exit(0)
    except BrokenPipeError:
        sys.exit(0)


if __name__ == "__main__":
    main()
