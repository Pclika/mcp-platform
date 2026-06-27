"""Fix PLACEHOLDER bug — restore main product images and rebuild extras correctly."""
import os, base64, glob, re

BASE = "L:/Pclika/website/mall/PlayersClub-main/src/data/products"

def main_svg(label, color='#f0f0f0'):
    svg = f'<svg xmlns="http://www.w3.org/2000/svg" width="400" height="500"><rect width="400" height="500" fill="#fafafa"/><rect x="0" y="0" width="400" height="500" fill="{color}" opacity="0.20"/><text x="200" y="240" text-anchor="middle" font-family="Arial,sans-serif" font-size="18" fill="#888" font-weight="700">{label}</text><line x1="40" y1="270" x2="360" y2="270" stroke="#ccc" stroke-width="1"/><text x="200" y="310" text-anchor="middle" font-family="monospace" font-size="12" fill="#bbb">PCLIKA</text></svg>'
    return "data:image/svg+xml;base64," + base64.b64encode(svg.encode()).decode()

def pinout_svg(label, color='#f0f0f0'):
    svg = f'<svg xmlns="http://www.w3.org/2000/svg" width="400" height="500"><rect width="400" height="500" fill="#fafafa"/><rect x="0" y="0" width="400" height="500" fill="{color}" opacity="0.18"/><text x="200" y="180" text-anchor="middle" font-family="Arial,sans-serif" font-size="13" fill="#777">Pinout</text><rect x="60" y="200" width="280" height="120" fill="none" stroke="#ccc" stroke-width="1"/><text x="200" y="230" text-anchor="middle" font-family="monospace" font-size="10" fill="#999">{label}</text><text x="200" y="252" text-anchor="middle" font-family="monospace" font-size="9" fill="#bbb">GND VCC SDA SCL TX RX</text><text x="200" y="340" text-anchor="middle" font-family="monospace" font-size="12" fill="#bbb">PCLIKA</text></svg>'
    return "data:image/svg+xml;base64," + base64.b64encode(svg.encode()).decode()

BUILD_SVG = '<svg xmlns="http://www.w3.org/2000/svg" width="400" height="500"><rect width="400" height="500" fill="#fafafa"/><rect x="40" y="40" width="320" height="420" fill="none" stroke="#ccc" stroke-width="1"/><text x="200" y="200" text-anchor="middle" font-family="Arial,sans-serif" font-size="13" fill="#777">Assembly View</text><rect x="100" y="230" width="200" height="100" fill="none" stroke="#999" stroke-width="1" stroke-dasharray="4"/><text x="200" y="280" text-anchor="middle" font-family="monospace" font-size="10" fill="#aaa">[ Mounting holes ]</text><text x="200" y="340" text-anchor="middle" font-family="monospace" font-size="12" fill="#bbb">PCLIKA</text></svg>'
BUILD_B64 = "data:image/svg+xml;base64," + base64.b64encode(BUILD_SVG.encode()).decode()

colors = {'Board':'#d0e8ff','Sensor':'#ffe4cc','Display':'#c8ffd4','Driver':'#e8d4ff','Bridge':'#ffd4d4','Power':'#fff8cc','Module':'#d4d4f0'}

for md_path in sorted(glob.glob(os.path.join(BASE, "*.md"))):
    with open(md_path, 'r', encoding='utf-8') as f:
        content = f.read()

    m_name = re.search(r'name:\s*"([^"]+)"', content)
    m_cat  = re.search(r'category:\s*"([^"]+)"', content)
    name = m_name.group(1) if m_name else "Product"
    cat  = m_cat.group(1) if m_cat else "Module"
    color = colors.get(cat, '#f0f0f0')

    main = main_svg(name[:22], color)
    pinout = pinout_svg(name[:18], color)

    # Fix the PLACEHOLDER in the main image src
    content = content.replace('src: "data:image/svg+xml;base64,PLACEHOLDER"', f'src: "{main}"')

    # Also fix extra images if they have PLACEHOLDER
    if 'PLACEHOLDER' in content:
        content = content.replace(
            '- src: "data:image/svg+xml;base64,PLACEHOLDER"',
            f'- src: "{pinout}"'
        )
        content = content.replace(
            '- src: "data:image/svg+xml;base64,PLACEHOLDER"',
            f'- src: "{BUILD_B64}"'
        )

    with open(md_path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"Fixed {os.path.basename(md_path)}")

# Verify no PLACEHOLDER left
for md_path in glob.glob(os.path.join(BASE, "*.md")):
    with open(md_path) as f:
        if 'PLACEHOLDER' in f.read():
            print(f"  STILL BROKEN: {os.path.basename(md_path)}")

print("Done")
