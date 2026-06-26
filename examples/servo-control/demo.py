#!/usr/bin/env python3
"""
demo.py — servo-control MCP demo

Connects to pclika-bridge via USB-Serial and exercises servo_move / servo_read.

Usage:
    python3 demo.py [--port /dev/ttyUSB0] [--baud 921600] [--sweep]

Options:
    --port PORT     Serial port (default: auto-detect)
    --baud BAUD     Baud rate (default: 921600)
    --sweep         Run continuous sweep instead of one-shot demo
"""

import argparse
import time
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "../../bridge/python"))
from pclika_bridge import SerialTransport, PClikaBridge


def run_demo(bridge: PClikaBridge, sweep: bool) -> None:
    # 1. Device info
    info = bridge.call("device_info", {})
    print(f"Device: {info.get('board_id')} caps={info.get('capabilities')}")

    if sweep:
        angles = [0, 45, 90, 135, 180, 135, 90, 45, 0]
        print("Sweep mode — Ctrl-C to stop")
        try:
            while True:
                for angle in angles:
                    result = bridge.call("servo_move", {
                        "channel":  0,
                        "angle":    angle,
                        "speed_ms": 300,
                    })
                    state = bridge.call("servo_read", {"channel": 0})
                    print(f"  → {angle:3d}°  status={result['status']}  "
                          f"current={state.get('angle')}°")
                    time.sleep(0.8)
        except KeyboardInterrupt:
            print("\nSweep stopped.")
    else:
        # One-shot: move to 0°, 90°, 180°
        for angle in [0, 90, 180]:
            result = bridge.call("servo_move", {
                "channel":  0,
                "angle":    angle,
                "speed_ms": 500,
            })
            time.sleep(0.6)
            state = bridge.call("servo_read", {"channel": 0})
            print(f"Move → {angle:3d}°  result={result['status']}  "
                  f"current={state.get('angle')}°  moving={state.get('moving')}")

    print("Demo complete.")


def main() -> None:
    parser = argparse.ArgumentParser(description="Pclika servo-control demo")
    parser.add_argument("--port",  default=None)
    parser.add_argument("--baud",  type=int, default=921600)
    parser.add_argument("--sweep", action="store_true")
    args = parser.parse_args()

    transport = SerialTransport(port=args.port, baud=args.baud)
    bridge    = PClikaBridge(transport)

    with bridge:
        run_demo(bridge, args.sweep)


if __name__ == "__main__":
    main()
