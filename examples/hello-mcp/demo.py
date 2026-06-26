#!/usr/bin/env python3
"""
hello-mcp demo script

Demonstrates calling Pclika MCP tools directly via the bridge transport.
Run this after flashing hello-mcp firmware and connecting your board.

Usage:
    pip install ./bridge/mcp-server
    python demo.py                          # auto-detect port
    python demo.py --port /dev/ttyUSB0     # specify port
    python demo.py --port COM3             # Windows

What this script does:
    1. Connects to the device
    2. Calls device_info  → prints board identity
    3. Calls firmware_version → prints firmware details
    4. Calls led_control (blink) → blinks LED 3 times via MCP
    5. Calls serial_log_read → prints recent device logs
"""

import sys
import time
import argparse

# Direct transport import (bypasses MCP STDIO layer for demo purposes)
sys.path.insert(0, "../../bridge/mcp-server")

from pclika_bridge.transport import SerialTransport
import json


def main():
    parser = argparse.ArgumentParser(description="hello-mcp demo")
    parser.add_argument("--port", help="Serial port (auto-detect if omitted)")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    print("=" * 60)
    print("  Pclika hello-mcp demo")
    print("  PCK-MMXXVI-C4A32096")
    print("=" * 60)

    t = SerialTransport(port=args.port, baud=args.baud)

    try:
        port = t.connect()
        print(f"\n✓ Connected on {port}\n")
    except Exception as e:
        print(f"\n✗ Connection failed: {e}")
        print("\nTips:")
        print("  • Confirm Pclika firmware is flashed")
        print("  • Check USB cable is data-capable (not charge-only)")
        print("  • Use --port to specify port manually")
        sys.exit(1)

    # ── Step 1: device_info ──────────────────────────────────────────────────
    print("─" * 40)
    print("1. device_info")
    print("─" * 40)
    try:
        info = t.command("device_info")
        print(json.dumps(info, indent=2))
    except Exception as e:
        print(f"Error: {e}")

    # ── Step 2: firmware_version ─────────────────────────────────────────────
    print("\n─" * 40)
    print("2. firmware_version")
    print("─" * 40)
    try:
        fwv = t.command("firmware_version")
        print(json.dumps(fwv, indent=2))
    except Exception as e:
        print(f"Error: {e}")

    # ── Step 3: led_control (blink 3 times) ──────────────────────────────────
    print("\n─" * 40)
    print("3. led_control — blink 3 times via MCP")
    print("─" * 40)
    for i in range(3):
        try:
            t.command("led_control", {"state": "on",  "r": 0, "g": 200, "b": 100})
            print(f"  [{i+1}/3] LED ON")
            time.sleep(0.4)
            t.command("led_control", {"state": "off"})
            print(f"  [{i+1}/3] LED OFF")
            time.sleep(0.4)
        except Exception as e:
            print(f"Error: {e}")
            break

    # ── Step 4: serial_log_read ───────────────────────────────────────────────
    print("\n─" * 40)
    print("4. serial_log_read — last 10 device log lines")
    print("─" * 40)
    try:
        logs = t.get_log_lines(10)
        for line in logs:
            print(f"  {line}")
        if not logs:
            print("  (no logs captured yet — try again in a moment)")
    except Exception as e:
        print(f"Error: {e}")

    t.disconnect()
    print("\n" + "=" * 60)
    print("  demo complete")
    print("=" * 60)
    print()
    print("Next steps:")
    print("  • Run pclika-bridge and import this repo into Claude or Codex")
    print("  • Call device_info, sensor_read, display_text via your AI tool")
    print("  • See examples/env-monitor for a sensor + display example")


if __name__ == "__main__":
    main()
