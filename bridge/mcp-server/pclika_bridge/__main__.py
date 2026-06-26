"""
pclika-bridge CLI entry point.

Usage:
  pclika-bridge                        # STDIO mode, auto-detect port
  pclika-bridge --port /dev/ttyUSB0   # STDIO mode, specify port
  pclika-bridge --port COM3            # STDIO mode, Windows
  pclika-bridge --serve                # HTTP/SSE server on localhost:3141
  pclika-bridge --serve --http-port 8080  # custom HTTP port
  pclika-bridge --baud 115200          # specify baud rate
  pclika-bridge --list-ports           # list available serial ports
  pclika-bridge --debug                # enable debug logging

Adaptive multi-client mode:
  Run once with --serve, then ALL clients (Claude, Codex, Cursor, VS Code,
  OpenCode) connect to http://localhost:3141/sse  without each spawning
  their own bridge process.

Setup helper:
  pclika-setup                         # auto-install configs for all detected clients
  pclika-setup --client cursor         # install for one client only
  pclika-setup --list                  # show detected clients
"""

from __future__ import annotations

import argparse
import logging
import sys

import serial.tools.list_ports


def main():
    parser = argparse.ArgumentParser(
        prog="pclika-bridge",
        description="Pclika MCP bridge — connects AI tools to Pclika hardware via USB",
    )
    parser.add_argument("--port",       metavar="PORT",  help="Serial port (auto-detect if omitted)")
    parser.add_argument("--baud",  "-b", type=int, default=115200, metavar="BAUD",
                        help="Baud rate (default: 115200)")
    parser.add_argument("--serve",      action="store_true",
                        help="Run as HTTP/SSE server (multi-client adaptive mode)")
    parser.add_argument("--http-port",  type=int, default=3141, metavar="PORT",
                        help="HTTP port for --serve mode (default: 3141)")
    parser.add_argument("--list-ports", action="store_true",
                        help="List available serial ports and exit")
    parser.add_argument("--debug",      action="store_true",
                        help="Enable debug logging")
    args = parser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(
        level=log_level,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        stream=sys.stderr,
    )
    log = logging.getLogger("pclika")

    if args.list_ports:
        ports = list(serial.tools.list_ports.comports())
        if not ports:
            print("No serial ports found.", file=sys.stderr)
        else:
            print("Available serial ports:", file=sys.stderr)
            for p in ports:
                vid = f"VID:{p.vid:04X}" if p.vid else "no VID"
                print(f"  {p.device:<20} {p.description} ({vid})", file=sys.stderr)
        sys.exit(0)

    from .transport import SerialTransport
    from .exceptions import DeviceNotConnected

    transport = SerialTransport(port=args.port, baud=args.baud)

    try:
        p