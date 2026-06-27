import express from "express";
import cors from "cors";
import mysql from "mysql2/promise";
import jwt from "jsonwebtoken";
import bcrypt from "bcryptjs";
import crypto from "crypto";

const app = express();
const PORT = 3001;
const JWT_SECRET = crypto.randomBytes(32).toString("hex");

app.use(cors({ origin: "https://mall.pclika.com", credentials: true }));
app.use(express.json());

// ── MySQL pool ──
const pool = mysql.createPool({
  host: "gz-cdb-a1i6th1p.sql.tencentcdb.com",
  port: 26870,
  user: "pclika_sylius",
  password: "rluZFgVNivwlSRmAw6r0KHyK",
  database: "Pclika",
  charset: "utf8mb4",
  waitForConnections: true,
  connectionLimit: 5,
});

// ── Helper: auth middleware ──
function auth(req, res, next) {
  const header = req.headers.authorization;
  if (!header) return res.status(401).json({ error: "No token" });
  try {
    const decoded = jwt.verify(header.replace("Bearer ", ""), JWT_SECRET);
    req.userId = decoded.id;
    req.userEmail = decoded.email;
    next();
  } catch {
    res.status(401).json({ error: "Invalid token" });
  }
}

// ── POST /api/register ──
app.post("/api/register", async (req, res) => {
  try {
    const { email, password, firstName, lastName } = req.body;
    if (!email || !password) return res.status(400).json({ error: "Email and password required" });

    const [existing] = await pool.query("SELECT id FROM sylius_shop_user WHERE username = ?", [email]);
    if (existing.length) return res.status(409).json({ error: "Email already registered" });

    const hashed = await bcrypt.hash(password, 12);

    // Create customer
    const [cust] = await pool.query(
      "INSERT INTO sylius_customer (email, email_canonical, first_name, last_name, subscribed_to_newsletter, created_at, updated_at) VALUES (?, ?, ?, ?, 0, NOW(), NOW())",
      [email, email.toLowerCase(), firstName || "", lastName || ""]
    );
    const customerId = cust.insertId;

    // Create shop user
    const [user] = await pool.query(
      "INSERT INTO sylius_shop_user (username, username_canonical, password, email, email_canonical, customer_id, enabled, roles, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, 1, '[]', NOW(), NOW())",
      [email, email.toLowerCase(), hashed, email, email.toLowerCase(), customerId]
    );

    const token = jwt.sign({ id: user.insertId, email }, JWT_SECRET, { expiresIn: "7d" });
    res.json({ token, user: { email, firstName, lastName } });
  } catch (err) {
    console.error("Register error:", err);
    res.status(500).json({ error: "Server error" });
  }
});

// ── POST /api/login ──
app.post("/api/login", async (req, res) => {
  try {
    const { email, password } = req.body;
    if (!email || !password) return res.status(400).json({ error: "Email and password required" });

    const [rows] = await pool.query(
      "SELECT su.id, su.password, su.username, su.customer_id, c.first_name, c.last_name FROM sylius_shop_user su LEFT JOIN sylius_customer c ON c.id = su.customer_id WHERE su.username = ?",
      [email]
    );
    if (!rows.length) return res.status(401).json({ error: "Invalid email or password" });

    const user = rows[0];

    // Try bcrypt first, then legacy argon2i
    let valid = false;
    try { valid = await bcrypt.compare(password, user.password); } catch {}
    if (!valid && user.password.startsWith("$argon2id$")) {
      try { valid = await bcrypt.compare(password, user.password); } catch {}
    }
    // Legacy Symfony encoded password fallback
    if (!valid) {
      const parts = user.password.split("$");
      if (parts.length === 4 && parts[1] === "2y" || parts[1] === "2b") {
        try { valid = await bcrypt.compare(password, user.password); } catch {}
      }
    }

    if (!valid) return res.status(401).json({ error: "Invalid email or password" });

    const token = jwt.sign({ id: user.id, email: user.username }, JWT_SECRET, { expiresIn: "7d" });
    res.json({ token, user: { email: user.username, firstName: user.first_name, lastName: user.last_name } });
  } catch (err) {
    console.error("Login error:", err);
    res.status(500).json({ error: "Server error" });
  }
});

// ── GET /api/me ──
app.get("/api/me", auth, async (req, res) => {
  try {
    const [rows] = await pool.query(
      "SELECT su.username, c.first_name, c.last_name FROM sylius_shop_user su LEFT JOIN sylius_customer c ON c.id = su.customer_id WHERE su.id = ?",
      [req.userId]
    );
    if (!rows.length) return res.status(404).json({ error: "User not found" });
    res.json({ user: rows[0] });
  } catch (err) {
    res.status(500).json({ error: "Server error" });
  }
});

// ── GET /api/products ──
app.get("/api/products", async (req, res) => {
  try {
    const [rows] = await pool.query(
      `SELECT p.id, p.code, pt.name, pt.slug, pt.description, pt.short_description
       FROM sylius_product p
       JOIN sylius_product_translation pt ON pt.translatable_id = p.id
       WHERE p.enabled = 1 AND pt.locale = 'en_US'
       ORDER BY p.id`
    );
    // Get cheapest price per product
    for (const product of rows) {
      const [prices] = await pool.query(
        `SELECT MIN(cp.price) as price FROM sylius_product_variant pv
         JOIN sylius_channel_pricing cp ON cp.product_variant_id = pv.id
         WHERE pv.product_id = ?`,
        [product.id]
      );
      product.price = prices[0]?.price ? `$${(prices[0].price / 100).toFixed(2)}` : null;
    }
    res.json({ products: rows });
  } catch (err) {
    console.error("Products error:", err);
    res.status(500).json({ error: "Server error" });
  }
});

// ── GET /api/products/:id ──
app.get("/api/products/:id", async (req, res) => {
  try {
    const [rows] = await pool.query(
      `SELECT p.id, p.code, pt.name, pt.slug, pt.description, pt.short_description
       FROM sylius_product p
       JOIN sylius_product_translation pt ON pt.translatable_id = p.id
       WHERE p.id = ? AND pt.locale = 'en_US'`,
      [req.params.id]
    );
    if (!rows.length) return res.status(404).json({ error: "Not found" });
    const [prices] = await pool.query(
      `SELECT MIN(cp.price) as price FROM sylius_product_variant pv
       JOIN sylius_channel_pricing cp ON cp.product_variant_id = pv.id
       WHERE pv.product_id = ?`,
      [rows[0].id]
    );
    rows[0].price = prices[0]?.price ? `$${(prices[0].price / 100).toFixed(2)}` : null;
    res.json({ product: rows[0] });
  } catch (err) {
    res.status(500).json({ error: "Server error" });
  }
});

// ── POST /api/cart ── (save cart - requires auth) ──
app.post("/api/cart", auth, async (req, res) => {
  try {
    const { items } = req.body; // [{productId, qty, price}]
    await pool.query("DELETE FROM pclika_cart WHERE user_id = ?", [req.userId]);
    for (const item of items) {
      await pool.query(
        "INSERT INTO pclika_cart (user_id, product_id, qty, price) VALUES (?, ?, ?, ?)",
        [req.userId, item.productId, item.qty || 1, item.price || 0]
      );
    }
    res.json({ ok: true });
  } catch (err) {
    res.status(500).json({ error: "Server error" });
  }
});

// ── GET /api/cart ── (requires auth) ──
app.get("/api/cart", auth, async (req, res) => {
  try {
    const [rows] = await pool.query(
      `SELECT c.product_id, c.qty, c.price, pt.name, pt.slug
       FROM pclika_cart c
       LEFT JOIN sylius_product_translation pt ON pt.translatable_id = c.product_id AND pt.locale = 'en_US'
       WHERE c.user_id = ?
       ORDER BY c.id`,
      [req.userId]
    );
    res.json({ items: rows });
  } catch (err) {
    res.status(500).json({ error: "Server error" });
  }
});

// ── POST /api/orders ── (requires auth) ──
app.post("/api/orders", auth, async (req, res) => {
  try {
    const [cartItems] = await pool.query(
      `SELECT c.* FROM pclika_cart c WHERE c.user_id = ?`,
      [req.userId]
    );
    if (!cartItems.length) return res.status(400).json({ error: "Cart is empty" });

    const orderId = crypto.randomUUID();
    const total = cartItems.reduce((s, i) => s + parseFloat(i.price || 0) * (i.qty || 1), 0);

    // Get or create customer number
    const [userRows] = await pool.query("SELECT customer_id FROM sylius_shop_user WHERE id = ?", [req.userId]);
    const [custRows] = await pool.query("SELECT id FROM sylius_customer WHERE id = ?", [userRows[0]?.customer_id]);

    await pool.query(
      `INSERT INTO pclika_orders (id, user_id, customer_id, total, status, created_at)
       VALUES (?, ?, ?, ?, 'pending', NOW())`,
      [orderId, req.userId, custRows[0]?.id || null, total.toFixed(2)]
    );

    for (const item of cartItems) {
      await pool.query(
        "INSERT INTO pclika_order_items (order_id, product_id, qty, price) VALUES (?, ?, ?, ?)",
        [orderId, item.product_id, item.qty, item.price]
      );
    }

    await pool.query("DELETE FROM pclika_cart WHERE user_id = ?", [req.userId]);
    res.json({ orderId, total: total.toFixed(2) });
  } catch (err) {
    console.error("Order error:", err);
    res.status(500).json({ error: "Server error" });
  }
});

// ── GET /api/orders ── (requires auth) ──
app.get("/api/orders", auth, async (req, res) => {
  try {
    const [rows] = await pool.query(
      "SELECT id, total, status, created_at FROM pclika_orders WHERE user_id = ? ORDER BY created_at DESC",
      [req.userId]
    );
    res.json({ orders: rows });
  } catch (err) {
    res.status(500).json({ error: "Server error" });
  }
});

// ── Start ──
app.listen(PORT, "127.0.0.1", () => {
  console.log(`PCLIKA API running on port ${PORT}`);
});
