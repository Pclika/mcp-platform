#!/usr/bin/env python3
"""
wifi-scanner demo — Pclika MCP Example
PCK-MMXXVI-C4A32096

Calls the wifi_scan tool and prints a sorted table of nearby APs.
"""
import argparse
import sys
from pclika_bridge import SerialTransport, PClikaBridge


def rssi_bar(rssi: int, width: int = 10) -> str:
    """Convert RSSI (-100 to -30 dBm) to ASCII bar."""
    clamped = max(-100, min(-30, rssi))
    ratio = (clamped + 100) / 70.0
    filled = int(ratio * width)
    return "█" * filled + "░" * (width - filled)


def security_icon(security: str) -> str:
    icons = {"OPEN": "🔓", "WEP": "⚠️ ", "WPA": "🔒", "WPA2": "🔒",
             "WPA3": "🔐", "WPA/WPA2": "🔒", "WPA2/WPA3": "🔐"}
    return icons.get(security, "?")


def main():
    parser = argparse.ArgumentParser(description="Pclika Wi-Fi Scanner Demo")
    parser.add_argument("--port",    default="/dev/ttyUSB0", help="Serial port")
    parser.add_argument("--baud",    type=int, default=115200)
    parser.add_argument("--max",     type=int, default=20, help="Max APs to return")
    parser.add_argument("--sort-by", choices=["rssi", "ssid", "channel"],
                        default="rssi", help="Sort column")
    parser.add_argument("--min-rssi", type=int, default=-100,
                        help="Filter APs with RSSI below this value")
    args = parser.parse_args()

    print(f"Connecting to {args.port} @ {args.baud} baud …")
    transport = SerialTransport(port=args.port, baudrate=args.baud)
    bridge    = PClikaBridge(transport)

    # Confirm connection
    info = bridge.call_tool("device_info")
    print(f"Device: {info.get('board_id')}  FW: {info.get('fw_version')}\n")

    print(f"Scanning for Wi-Fi networks (max {args.max}) …")
    result = bridge.call_tool("wifi_scan", {"max_results": args.max})

    if "error" in result:
        print(f"ERROR: {result['error']}", file=sys.stderr)
        sys.exit(1)

    networks = result.get("networks", [])
    scan_ms  = result.get("scan_time_ms", 0)

    # Filter by RSSI
    networks = [n for n in networks if n.get("rssi", -200) >= args.min_rssi]

    # Sort
    if args.sort_by == "rssi":
        networks.sort(key=lambda n: n.get("rssi", -200), reverse=True)
    elif args.sort_by == "ssid":
        networks.sort(key=lambda n: n.get("ssid", "").lower())
    elif args.sort_by == "channel":
        networks.sort(key=lambda n: n.get("channel", 0))

    print(f"{'#':<3} {'SSID':<32} {'RSSI':>5}  {'Signal':10}  {'Ch':>2}  {'Security':<12}  {'BSSID'}")
    print("─" * 90)
    for idx, ap in enumerate(networks, 1):
        ssid     = ap.get("ssid", "")[:32] or "<hidden>"
        rssi     = ap.get("rssi", 0)
        channel  = ap.get("channel", 0)
        security = ap.get("security", "?")
        bssid    = ap.get("bssid", "")
        bar      = rssi_bar(rssi)
        icon     = security_icon(security)
        print(f"{idx:<3} {ssid:<32} {rssi:>5}  {bar:10}  {channel:>2}  "
              f"{icon} {security:<10}  {bssid}")

    print(f"\nFound {len(networks)} AP(s)  |  scan time: {scan_ms} ms")
    transport.close()


if __name__ == "__main__":
    main()
