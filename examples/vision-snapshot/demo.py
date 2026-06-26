#!/usr/bin/env python3
"""
vision-snapshot demo — Pclika MCP Example
PCK-MMXXVI-C4A32096

Captures a JPEG snapshot via camera_capture and saves it locally.
"""
import argparse
import base64
import sys
import time
from pathlib import Path
from pclika_bridge import SerialTransport, PClikaBridge


def main():
    parser = argparse.ArgumentParser(description="Pclika Vision Snapshot Demo")
    parser.add_argument("--port",        default="/dev/ttyUSB0")
    parser.add_argument("--baud",        type=int, default=115200)
    parser.add_argument("--resolution",  default="VGA",
                        choices=["QVGA", "VGA", "SVGA", "XGA", "HD", "SXGA", "UXGA"])
    parser.add_argument("--quality",     type=int, default=20)
    parser.add_argument("--output",      default="snapshot.jpg", help="Output filename")
    parser.add_argument("--status",      action="store_true", help="Print camera status only")
    parser.add_argument("--continuous",  type=int, default=0, metavar="N",
                        help="Capture N frames continuously")
    args = parser.parse_args()

    print(f"Connecting to {args.port} …")
    transport = SerialTransport(port=args.port, baudrate=args.baud)
    bridge    = PClikaBridge(transport)

    info = bridge.call_tool("device_info")
    print(f"Device: {info.get('board_id')}  FW: {info.get('fw_version')}")

    if args.status:
        status = bridge.call_tool("camera_status")
        print(f"\nCamera status:")
        for k, v in status.items():
            print(f"  {k}: {v}")
        transport.close()
        return

    n_frames = max(1, args.continuous)
    for frame_idx in range(n_frames):
        print(f"Capturing frame {frame_idx + 1}/{n_frames} "
              f"({args.resolution} q={args.quality}) …", end=" ", flush=True)

        result = bridge.call_tool("camera_capture", {
            "resolution": args.resolution,
            "quality":    args.quality,
        })

        if not result.get("ok"):
            print(f"FAILED: {result.get('error', 'unknown')}", file=sys.stderr)
            sys.exit(1)

        jpeg_data = base64.b64decode(result["base64"])
        width     = result.get("width", 0)
        height    = result.get("height", 0)
        size_kb   = len(jpeg_data) / 1024

        if n_frames > 1:
            out_path = Path(args.output)
            out_path = out_path.with_stem(f"{out_path.stem}_{frame_idx:04d}")
        else:
            out_path = Path(args.output)

        out_path.write_bytes(jpeg_data)
        print(f"{width}×{height}  {size_kb:.1f} KB  → {out_path}")

        if args.continuous > 1 and frame_idx < n_frames - 1:
            time.sleep(0.1)

    print(f"\nDone.")
    transport.close()


if __name__ == "__main__":
    main()
