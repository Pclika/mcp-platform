#!/usr/bin/env python3
"""
industrial-gateway demo — Pclika MCP Example
PCK-MMXXVI-C4A32096

Demonstrates Modbus read, relay control, ADC, and digital I/O via MCP.
"""
import argparse
import sys
import time
from pclika_bridge import SerialTransport, PClikaBridge


def demo_modbus(bridge: "PClikaBridge", slave: int):
    print(f"\n[Modbus] Reading 5 holding registers from slave {slave} @ addr 0 …")
    result = bridge.call_tool("modbus_read", {
        "slave_id": slave,
        "function": "holding_registers",
        "address":  0,
        "count":    5,
    })
    if result.get("ok"):
        vals = result.get("values", [])
        print(f"  Registers: {vals}")
    else:
        print(f"  ERROR: {result.get('error', 'no response')}")


def demo_relay(bridge: "PClikaBridge"):
    print("\n[Relay] Cycling relays 0 and 1 …")
    for ch in [0, 1]:
        r = bridge.call_tool("relay_set", {"channel": ch, "state": "on"})
        print(f"  CH{ch} → {r.get('state')}  all: {r.get('all_states', [])[:4]}…")
        time.sleep(0.3)
    time.sleep(0.5)
    r = bridge.call_tool("relay_set", {"channel": -1, "state": "off"})
    print(f"  All OFF → {r.get('all_states')}")


def demo_adc(bridge: "PClikaBridge"):
    print("\n[ADC] Reading 4 channels …")
    result = bridge.call_tool("adc_read_multi", {"channels": [0, 1, 2, 3]})
    for reading in result.get("readings", []):
        ch  = reading.get("channel", 0)
        raw = reading.get("raw", 0)
        v   = reading.get("voltage", 0.0)
        bar = "█" * int(v / 3.3 * 20) + "░" * (20 - int(v / 3.3 * 20))
        print(f"  CH{ch}: raw={raw:4d}  {v:.3f} V  [{bar}]")


def demo_digital_io(bridge: "PClikaBridge"):
    print("\n[Digital I/O] Reading input bank …")
    result = bridge.call_tool("digital_io_read", {})
    levels = result.get("levels", [])
    mask   = result.get("mask", 0)
    print(f"  Levels: {levels}")
    print(f"  Mask:   0b{mask:08b}  (0x{mask:02X})")


def main():
    parser = argparse.ArgumentParser(description="Pclika Industrial Gateway Demo")
    parser.add_argument("--port",       default="/dev/ttyUSB0")
    parser.add_argument("--baud",       type=int, default=115200)
    parser.add_argument("--slave",      type=int, default=1, help="Modbus slave ID")
    parser.add_argument("--skip-modbus",action="store_true",
                        help="Skip Modbus demo (no RS-485 device connected)")
    parser.add_argument("--loop",       type=int, default=1,
                        help="Repeat demo N times")
    args = parser.parse_args()

    print(f"Connecting to {args.port} …")
    transport = SerialTransport(port=args.port, baudrate=args.baud)
    bridge    = PClikaBridge(transport)

    info = bridge.call_tool("device_info")
    print(f"Device: {info.get('board_id')}  FW: {info.get('fw_version')}")

    for iteration in range(args.loop):
        if args.loop > 1:
            print(f"\n══ Iteration {iteration + 1}/{args.loop} ══")

        if not args.skip_modbus:
            demo_modbus(bridge, args.slave)

        demo_relay(bridge)
        demo_adc(bridge)
        demo_digital_io(bridge)

        if args.loop > 1 and iteration < args.loop - 1:
            time.sleep(1.0)

    print("\nDone.")
    transport.close()


if __name__ == "__main__":
    main()
