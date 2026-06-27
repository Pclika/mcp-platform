# hc-server — Pclika MCP Platform Cloud Server

MCP 1.28.0 FastMCP backend for `hc.pclika.com`.  
Bridges AI clients (Claude, Cursor, VS Code) to Pclika ESP32-S3 devices in real time.

## Architecture

```
AI Client (Claude/Cursor)
   │  GET /sse  or  POST /mcp
   ▼
hc-server  ──── WS /device ────  Pclika Board (ESP32-S3)
   │
  keys.json (API keys)
```

Each API key maps to one physical device's WebSocket connection.  
If no device is connected, tools return `_demo: true` simulation data.

## Quick start (Docker)

```bash
# 1. Set your admin key
export ADMIN_KEY=your-secret-admin-key

# 2. Start
docker compose up -d

# 3. Health check
curl https://hc.pclika.com/health

# 4. List tools (no auth)
curl https://hc.pclika.com/mcp/tools

# 5. Create an API key
curl -X POST https://hc.pclika.com/admin/keys \
  -H "X-Admin-Key: $ADMIN_KEY" \
  -H "Content-Type: application/json" \
  -d '{"label":"My Device"}'
# → {"key": "pck_a1b2c3..."}

# 6. Test MCP via demo (no device needed)
curl https://hc.pclika.com/mcp/tools \
  -H "X-API-Key: pck_demo00000000000000000000000000000"
```

## Claude Code / Claude Desktop integration

```bash
# Add MCP server to Claude Code
claude mcp add pclikaPlatform \
  --transport sse \
  https://hc.pclika.com/sse \
  --header "X-API-Key: pck_YOUR_KEY"
```

Or copy `configs/mcp/claude-code.commands.md` from the repo.

## Device WebSocket protocol (for firmware)

Device connects to `wss://hc.pclika.com/device` and exchanges JSON messages:

**Step 1 — Register**
```json
→ {"type":"register","api_key":"pck_xxx","board_id":"PCK-001","fw":"0.1.0"}
← {"type":"registered","board_id":"PCK-001"}
```

**Step 2 — Receive tool calls**
```json
← {"type":"tool_call","id":"uuid","name":"sensor_read","args":{"sensor_id":"temp_humidity"}}
→ {"type":"tool_result","id":"uuid","result":{"value":{"temperature":24.5,"humidity":58.2},"unit":"%RH/°C","timestamp_ms":12345}}
```

**Error response**
```json
→ {"type":"tool_error","id":"uuid","error":"I2C read failed"}
```

## Nginx config (for hc.pclika.com)

```nginx
server {
    listen 443 ssl;
    server_name hc.pclika.com;

    # SSL via certbot / Let's Encrypt
    ssl_certificate     /etc/letsencrypt/live/hc.pclika.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/hc.pclika.com/privkey.pem;

    location / {
        proxy_pass http://127.0.0.1:8080;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;

        # SSE: disable buffering
        proxy_buffering off;
        proxy_cache off;
        proxy_read_timeout 3600s;
    }
}
```

## Environment variables

| Variable   | Default      | Description                            |
|-----------|-------------|----------------------------------------|
| `PORT`     | `8080`       | Listen port                            |
| `KEYS_FILE`| `keys.json`  | Path to API keys JSON file             |
| `ADMIN_KEY`| (required)   | Admin key for `/admin/keys` endpoints  |

## Endpoints

| Method | Path             | Auth      | Description                         |
|--------|-----------------|-----------|-------------------------------------|
| GET    | `/health`        | None      | Liveness probe                      |
| GET    | `/mcp/tools`     | None      | Tool list                           |
| GET    | `/sse`           | X-API-Key | MCP SSE transport                   |
| POST   | `/mcp`           | X-API-Key | MCP streamable HTTP transport       |
| WS     | `/device`        | In message| Device registration WebSocket       |
| GET    | `/admin/keys`    | X-Admin-Key | List API keys                    |
| POST   | `/admin/keys`    | X-Admin-Key | Create API key                   |
| DELETE | `/admin/keys/:k` | X-Admin-Key | Revoke API key                   |
