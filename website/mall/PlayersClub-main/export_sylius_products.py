#!/usr/bin/env python3
"""
Export Sylius hardware products → Astro markdown files for Players Club site.

Usage:
  python export_sylius_products.py
"""

import pymysql
import os
import re

# ── DB config ──
DB_HOST = "gz-cdb-a1i6th1p.sql.tencentcdb.com"
DB_PORT = 26870
DB_USER = "pclika_sylius"
DB_PASS = "rluZFgVNivwlSRmAw6r0KHyK"
DB_NAME = "Pclika"

# ── Taxons → Astro category mapping ──
TAXON_TO_CATEGORY = {
    13: "Board",
    14: "Sensor",
    15: "Display",
    16: "Driver",
    17: "Bridge",
    18: "Power",
    19: "Module",
}

# ── Connection ──
conn = pymysql.connect(
    host=DB_HOST, port=DB_PORT, user=DB_USER, password=DB_PASS, database=DB_NAME,
    charset="utf8mb4", cursorclass=pymysql.cursors.DictCursor
)
cur = conn.cursor()


def slug_to_id(slug: str) -> str:
    """Convert Sylius slug to Astro file ID (basename without ext)."""
    return slug.rsplit("/", 1)[-1]


def cents_to_price(cents: int) -> str:
    return f"${cents / 100:.2f}"


def pick_category(product_id: int) -> str:
    cur.execute(
        "SELECT taxon_id FROM sylius_product_taxon WHERE product_id = %s",
        (product_id,)
    )
    taxon_ids = [r["taxon_id"] for r in cur.fetchall()]
    for tid in [13, 14, 15, 16, 17, 18, 19]:
        if tid in taxon_ids:
            return TAXON_TO_CATEGORY[tid]
    return "Module"


def product_specs_from_description(desc: str) -> dict:
    """Parse structured specs from Sylius description."""
    return {}


def generate_markdown(product, desc, specs: dict, price: str):
    """Generate Astro markdown frontmatter + body."""
    pid = slug_to_id(product["slug"])

    lines = ["---"]
    lines.append(f'name: "{product["name"]}"')
    lines.append(f'sku: "{product["code"]}"')
    lines.append(f'category: "{pick_category(product["id"])}"')
    lines.append(f'price: "{price}"')
    lines.append("image:")
    lines.append(f'  src: "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MDAiIGhlaWdodD0iNTAwIj48cmVjdCB3aWR0aD0iNDAwIiBoZWlnaHQ9IjUwMCIgZmlsbD0iI2ZhZmFmYSIvPjx0ZXh0IHg9IjIwMCIgeT0iMjQwIiB0ZXh0LWFuY2hvcj0ibWlkZGxlIiBmb250LWZhbWlseT0iQXJpYWwsc2Fucy1zZXJpZiIgZm9udC1zaXplPSIxOCIgZmlsbD0iIzg4OCI+{product["name"]}</text></svg>"')
    lines.append(f'  alt: "{product["name"]}"')
    lines.append("images:")
    lines.append(f'  - src: "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MDAiIGhlaWdodD0iNTAwIj48cmVjdCB3aWR0aD0iNDAwIiBoZWlnaHQ9IjUwMCIgZmlsbD0iI2ZhZmFmYSIvPjx0ZXh0IHg9IjIwMCIgeT0iMTgwIiB0ZXh0LWFuY2hvcj0ibWlkZGxlIiBmb250LWZhbWlseT0iQXJpYWwsc2Fucy1zZXJpZiIgZm9udC1zaXplPSIxMyIgZmlsbD0iIzc3NyI+{product["name"]} — Pinout</text></svg>"')
    lines.append(f'    alt: "{product["name"]} — Pinout"')
    lines.append("specs:")
    for key in ("chip", "voltage", "protocol", "interface", "pins", "resolution", "channels", "dimensions", "weight"):
        val = specs.get(key, "--")
        # Escape any quotes
        val = val.replace('"', "'")
        lines.append(f'  {key}: "{val}"')
    lines.append("---\n")
    lines.append(desc if desc else "")
    lines.append("")

    return "\n".join(lines)


def main():
    output_dir = os.path.join(os.path.dirname(__file__), "..", "src", "data", "products")
    os.makedirs(output_dir, exist_ok=True)

    # Fetch hardware products (IDs 88+)
    cur.execute("""
        SELECT p.id, p.code, pt.name, pt.slug, pt.description
        FROM sylius_product p
        JOIN sylius_product_translation pt ON pt.translatable_id = p.id
        WHERE p.id >= 88 AND p.enabled = 1 AND pt.locale = 'en_US'
        ORDER BY p.id
    """)
    products = cur.fetchall()

    # Fetch pricing
    cur.execute("""
        SELECT pv.product_id, MIN(cp.price) AS min_price
        FROM sylius_product_variant pv
        JOIN sylius_channel_pricing cp ON cp.product_variant_id = pv.id
        WHERE pv.product_id >= 88
        GROUP BY pv.product_id
    """)
    pricing_rows = cur.fetchall()
    prices = {r["product_id"]: r["min_price"] for r in pricing_rows}

    generated = []
    for product in products:
        pid = product["id"]
        price = cents_to_price(prices.get(pid, 0))
        desc = product["description"] or ""
        specs = product_specs_from_description(desc)
        md = generate_markdown(product, desc, specs, price)

        file_id = slug_to_id(product["slug"])
        file_path = os.path.join(output_dir, f"{file_id}.md")

        with open(file_path, "w", encoding="utf-8") as f:
            f.write(md)

        generated.append((file_id, product["name"], price))

    print(f"\n✅ Generated {len(generated)} product files:\n")
    for fid, name, price in generated:
        print(f"  {fid:40s}  {name:30s}  {price}")

    cur.close()
    conn.close()


if __name__ == "__main__":
    main()
