#!/usr/bin/env bash
# export_gerbers.sh — Pclika ESP32-S3 v0.1 Manufacturing File Export
# Usage:
#   ./export_gerbers.sh                          # auto-detect kicad-cli
#   ./export_gerbers.sh /path/to/kicad-cli       # explicit path
#
# Platform paths for kicad-cli:
#   macOS  : /Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli
#   Linux  : kicad-cli  (if kicad package installed)
#   Windows: "C:/Program Files/KiCad/8.0/bin/kicad-cli.exe"  (use Git Bash)
#
# Output layout:
#   manufacturing/
#   ├── gerbers/          ← Gerber + drill files (zip this for JLCPCB)
#   │   ├── *.GTL GTB GTO GTS GTD GBL GBO GTS GBS GBD GM1 GKO DRL
#   │   └── *.drl  *.gbr (drill map)
#   └── bom/
#       ├── jlcpcb_bom.csv           ← upload to JLCPCB SMT Assembly
#       └── component_placement_top.csv  ← upload to JLCPCB SMT Assembly

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT="$SCRIPT_DIR/pclika-esp32s3-v01"
PCB="$PROJECT.kicad_pcb"
OUT="$SCRIPT_DIR/manufacturing"
GERBER_OUT="$OUT/gerbers"
BOM_OUT="$OUT/bom"

# ── Find kicad-cli ─────────────────────────────────────────────────────────────
if [ -n "${1:-}" ]; then
  KICAD_CLI="$1"
else
  # Auto-detect common locations
  for candidate in \
    "kicad-cli" \
    "/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli" \
    "/usr/bin/kicad-cli" \
    "/usr/local/bin/kicad-cli" \
    "C:/Program Files/KiCad/8.0/bin/kicad-cli.exe"; do
    if command -v "$candidate" &>/dev/null 2>&1 || [ -f "$candidate" ]; then
      KICAD_CLI="$candidate"
      break
    fi
  done
fi

if [ -z "${KICAD_CLI:-}" ] || ! (command -v "$KICAD_CLI" &>/dev/null 2>&1 || [ -f "$KICAD_CLI" ]); then
  echo "ERROR: kicad-cli not found."
  echo "  Install KiCad 8 from https://www.kicad.org/download/"
  echo "  Or pass path: ./export_gerbers.sh /path/to/kicad-cli"
  exit 1
fi

echo "============================================="
echo " Pclika ESP32-S3 v0.1 — Manufacturing Export"
echo "============================================="
echo " kicad-cli : $KICAD_CLI"
echo " PCB file  : $PCB"
echo " Output    : $OUT"
echo ""

# Pre-flight: check PCB file exists and is non-empty
if [ ! -f "$PCB" ]; then
  echo "ERROR: PCB file not found: $PCB"
  echo "  Complete routing in KiCad 8 first (see ROUTING_GUIDE.md)"
  exit 1
fi

mkdir -p "$GERBER_OUT" "$BOM_OUT"

# ── 1. Gerbers ─────────────────────────────────────────────────────────────────
echo "[1/5] Exporting Gerbers (JLCPCB layer naming)..."
"$KICAD_CLI" pcb export gerbers "$PCB" \
  --output "$GERBER_OUT" \
  --layers "F.Cu,B.Cu,In1.Cu,In2.Cu,F.Paste,B.Paste,F.Silkscreen,B.Silkscreen,F.Mask,B.Mask,Edge.Cuts" \
  --use-protel-extensions \
  --no-protel-x2-attributes \
  --subtract-soldermask-from-silkscreen \
  --board-plot-params

# ── 2. Drill files ─────────────────────────────────────────────────────────────
echo "[2/5] Exporting Drill files..."
"$KICAD_CLI" pcb export drill "$PCB" \
  --output "$GERBER_OUT/" \
  --format excellon \
  --drill-origin absolute \
  --excellon-zeros-format decimal \
  --excellon-oval-format route \
  --excellon-units mm \
  --generate-map \
  --map-format gerberx2

# ── 3. BOM ─────────────────────────────────────────────────────────────────────
echo "[3/5] Copying JLCPCB BOM..."
cp "$SCRIPT_DIR/jlcpcb_bom.csv" "$BOM_OUT/"

# ── 4. Placement ───────────────────────────────────────────────────────────────
echo "[4/5] Copying placement CSV..."
cp "$SCRIPT_DIR/component_placement_top.csv" "$BOM_OUT/"

# ── 5. Zip Gerbers ─────────────────────────────────────────────────────────────
echo "[5/5] Zipping Gerbers for JLCPCB upload..."
cd "$GERBER_OUT"
ZIPNAME="pclika-esp32s3-v01-gerbers.zip"
zip -r "../$ZIPNAME" . -x "*.DS_Store"
cd "$SCRIPT_DIR"
echo "       → $OUT/$ZIPNAME"

# ── Summary ────────────────────────────────────────────────────────────────────
echo ""
echo "============================================="
echo " Export complete!"
echo "============================================="
echo ""
echo "Gerber zip : $OUT/pclika-esp32s3-v01-gerbers.zip"
echo "BOM        : $BOM_OUT/jlcpcb_bom.csv"
echo "Placement  : $BOM_OUT/component_placement_top.csv"
echo ""
echo "JLCPCB 下单步骤："
echo "  1. jlcpcb.com → Instant Quote → Upload Gerber ZIP"
echo "  2. 板材设置：FR-4 / 65×45mm / 4层 / 1.6mm / 黑色油墨 / ENIG (推荐) / 数量50"
echo "  3. 勾选 SMT Assembly → Top Side"
echo "  4. 上传 jlcpcb_bom.csv 和 component_placement_top.csv"
echo "  5. 确认元件匹配（主要检查 U1 ESP32 模组和 U4 CH340C）"
echo "  6. 下单前核对 3D 预览"
echo ""
echo "详细步骤见：hardware/kicad/JLCPCB_ORDER_GUIDE.md"
