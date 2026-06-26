#!/usr/bin/env python3
"""
env-monitor demo script

Demonstrates sensor_read + display_text MCP tools on a board
running env-monitor firmware (DHT22 + SSD1306 OLED).

Usage:
    pip install ./bridge/mcp-server
    python demo.py                          # auto-detect port
    python demo.py --port /dev/ttyUSB0     # specify port
    python demo.py --port COM3             # Windows
    python demo.py --loop 10              # poll every 10 seconds
"""

import sys
import time
import argparse
import json

sys.path.insert(0, "../../bridge/mcp-server")
from pclika_bridge.transport import SerialTransport


SEAL = "PCK-MMXXVI-C4A32096"


def fmt_env(data: dict) -> str:
    temp = data.get("value", "?")
    hum  = data.get("value2", "?")
    unit = data.get("unit", "")
    unit2 = data.get("unit2", "")
    return f"{temp} {unit}  /  {hum} {unit2}"


def main():
    parser = argparse.ArgumentParser(description="env-monitor MCP demo")
    parser.add_argument("--port",  help="Serial port (auto-detect if omitted)")
    parser.add_argument("--baud",  type=int, default=115200)
    parser.add_argument("--loop",  type=int, default=0,
                        help="Poll interval in seconds (0 = read once and exit)")
    args = parser.parse_args()

    print("=" * 60)
    print("  Pclika env-monitor demo")
    print(f"  {SEAL}")
    print("=" * 60)

    t = SerialTransport(port=args.port, baud=args.baud)

    try:
        port = t.connect()
        print(f"\n✓ Connected on {port}\n")
    except Exception as e:
        print(f"\n✗ Connection failed: {e}")
        print("\nTips:")
        print("  • Flash env-monitor firmware first: idf.py flash")
        print("  • Check USB cable (data-capable, not charge-only)")
        print("  • Use --port to specify port manually")
        sys.exit(1)

    # ── Step 1: device_info ──────────────────────────────────────────────────
    print("─" * 40)
    print("1. device_info")
    print("─" * 40)
    try:
        info = t.command("device_info")
        caps = info.get("capabilities", {})
        print(f"  Board ID  : {info.get('board_id', '?')}")
        print(f"  Firmware  : {info.get('firmware', '?')}")
        print(f"  Platform  : {info.get('platform', '?')}")
        print(f"  Seal      : {info.get('seal', '?')}")
        print(f"  has_sensor: {caps.get('has_sensor', False)}")
        print(f"  has_display:{caps.get('has_display', False)}")
    except Exception as e:
        print(f"  Error: {e}")

    # ── Step 2: sensor_read ──────────────────────────────────────────────────
    def read_sensor():
        print("\n─" * 40)
        print("2. sensor_read(sensor_id=\"temp_humidity\")")
        print("─" * 40)
        try:
            result = t.command("sensor_read", {"sensor_id": "temp_humidity"})
            if result.get("valid"):
                env_str = fmt_env(result)
                print(f"  ✓ {env_str}")
                print(f"  Timestamp: {result.get('timestamp_us', 0) // 1_000_000}s uptime")
                return result
            else:
                print(f"  ✗ Sensor error: {result.get('error', 'unknown')}")
        except Exception as e:
            print(f"  Error: {e}")
        return None

    # ── Step 3: display_text ─────────────────────────────────────────────────
    def update_display(reading):
        if not reading:
            return
        print("\n─" * 40)
        print("3. display_text — update OLED")
        print("─" * 40)
        temp  = reading.get("value", 0)
        hum   = reading.get("value2", 0)
        line0 = f"Temp: {temp:.1f} C"
        line1 = f"Hum:  {hum:.1f} %"
        try:
            t.command("display_text", {"text": line0, "line": 0, "clear": True})
            print(f"  → OLED line 0: \"{line0}\"")
            t.command("display_text", {"text": line1, "line": 1})
            print(f"  → OLED line 1: \"{line1}\"")
        except Exception as e:
            print(f"  Error: {e}")

    # ── Run ──────────────────────────────────────────────────────────────────
    if args.loop > 0:
        print(f"\nPolling every {args.loop}s — Ctrl+C to stop\n")
        try:
            iteration = 0
            while True:
                iteration += 1
                print(f"\n[{iteration}] {time.strftime('%H:%M:%S')}")
                reading = read_sensor()
                update_display(reading)
                time.sleep(args.loop)
        except KeyboardInterrupt:
            print("\nStopped.")
    else:
        reading = read_sensor()
        update_display(reading)

        # ── Step 4: serial_log_read ──────────────────────────────────────────
        print("\n─" * 40)
        print("4. serial_log_read — last 15 device log lines")
        print("─" * 40)
        try:
            logs = t.get_log_lines(15)
            for line in logs:
                print(f"  {line}")
            if not logs:
                print("  (no logs yet — try again after a few seconds)")
        except Exception as e:
            print(f"  Error: {e}")

    t.disconnect()
    print("\n" + "=" * 60)
    print("  demo complete")
    print("=" * 60)
    print()
    print("To use via Claude or Codex MCP:")
    print('  sensor_read(sensor_id="temp_humidity")')
    print('  display_text(text="Hello Claude", line=0, clear=true)')
    print('  serial_log_read(lines=20)')


if __name__ == "__main__":
    main()
