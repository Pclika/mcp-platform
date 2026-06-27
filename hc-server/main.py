"""
hc.pclika.com — Pclika MCP Platform Cloud Server  v0.2.0
FastMCP 1.28.0 + Starlette

Public:   GET /health   GET /mcp/tools
MCP:      GET /sse      POST /mcp          (X-API-Key)
Device:   WS  /device                      (api_key in message)
Admin:    /admin/*                          (X-Admin-Key)
"""
import contextvars
import json
import logging
import os
import time
from pathlib import Path

import uvicorn
from mcp.server.fastmcp import FastMCP, Context
from starlette.applications import Starlette
from starlette.middleware import Middleware
from starlette.middleware.base import BaseHTTPMiddleware
from starlette.middleware.cors import CORSMiddleware
from starlette.requests import Request
from starlette.responses import FileResponse, JSONResponse
from starlette.routing import Mount, Route, WebSocketRoute
from starlette.staticfiles import StaticFiles
from starlette.websockets import WebSocket, WebSocketDisconnect

from auth import AuthManager, DEMO_KEY
from device_manager import DeviceManager, call_logger

logging.basicConfig(level=logging.INFO,
                    format="%(asctime)s %(levelname)-7s %(name)s │ %(message)s")
logger = logging.getLogger("hc-server")

auth = AuthManager()
device_mgr = DeviceManager()
ADMIN_KEY = os.environ.get("ADMIN_KEY", "")
_current_key: contextvars.ContextVar[str] = contextvars.ContextVar("current_key", default=DEMO_KEY)

# ── FastMCP + 11 tools ─────────────────────────────────────────────────────────
mcp = FastMCP(name="pclikaPlatform", stateless_http=False,
              instructions="Call device_info first. Results with _demo:true mean no device connected.")

@mcp.tool(description="Return board identity, firmware version, platform seal and capability flags.")
async def device_info(ctx: Context) -> dict:
    return await device_mgr.call(_current_key.get(), "device_info", {})

@mcp.tool(description="Read a sensor value. sensor_id: 'temp_humidity','pressure','light','imu','distance','co2'.")
async def sensor_read(sensor_id: str, channel: int = 0) -> dict:
    return await device_mgr.call(_current_key.get(), "sensor_read", {"sensor_id": sensor_id, "channel": channel})

@mcp.tool(description="Render text on the connected OLED/TFT display.")
async def display_text(text: str, line: int = 0, clear: bool = False, display_id: str = "primary") -> dict:
    return await device_mgr.call(_current_key.get(), "display_text",
                                  {"text": text, "line": line, "clear": clear, "display_id": display_id})

@mcp.tool(description="Move a servo to the specified angle (0-180 degrees).")
async def servo_move(angle: float, channel: int = 0, speed: int = 100) -> dict:
    return await device_mgr.call(_current_key.get(), "servo_move",
                                  {"angle": angle, "channel": channel, "speed": speed})

@mcp.tool(description="Return recent serial log output from the device.")
async def serial_log_read(lines: int = 50, filter: str = "") -> dict:
    return await device_mgr.call(_current_key.get(), "serial_log_read", {"lines": lines, "filter": filter})

@mcp.tool(description="Control onboard LED or addressable RGB LEDs. state: 'on'|'off'|'blink'.")
async def led_control(state: str, r: int = 255, g: int = 255, b: int = 255,
                      brightness: int = 128, blink_hz: float = 1.0) -> dict:
    return await device_mgr.call(_current_key.get(), "led_control",
                                  {"state": state, "r": r, "g": g, "b": b,
                                   "brightness": brightness, "blink_hz": blink_hz})

@mcp.tool(description="Read the state of the onboard or external button.")
async def button_read(button_id: str = "boot", reset_count: bool = False) -> dict:
    return await device_mgr.call(_current_key.get(), "button_read",
                                  {"button_id": button_id, "reset_count": reset_count})

@mcp.tool(description="Scan for nearby Wi-Fi access points.")
async def wifi_scan(max_results: int = 20) -> dict:
    return await device_mgr.call(_current_key.get(), "wifi_scan", {"max_results": max_results})

@mcp.tool(description="Read the digital level of a GPIO pin. pull: 'up'|'down'|'none'.")
async def gpio_read(pin: int, pull: str = "none") -> dict:
    return await device_mgr.call(_current_key.get(), "gpio_read", {"pin": pin, "pull": pull})

@mcp.tool(description="Set the digital output level of a GPIO pin. level: 0 or 1.")
async def gpio_write(pin: int, level: int) -> dict:
    return await device_mgr.call(_current_key.get(), "gpio_write", {"pin": pin, "level": level})

@mcp.tool(description="Return firmware version, build date, platform seal, and supported tool list.")
async def firmware_version() -> dict:
    return await device_mgr.call(_current_key.get(), "firmware_version", {})


# ── Public handlers ────────────────────────────────────────────────────────────
TOOL_LIST = [
    {"name": "device_info",      "description": "Board identity, firmware, capabilities"},
    {"name": "sensor_read",      "description": "Read sensor value by sensor_id"},
    {"name": "display_text",     "description": "Render text on OLED/TFT display"},
    {"name": "servo_move",       "description": "Move servo to angle (0-180°)"},
    {"name": "serial_log_read",  "description": "Fetch recent device serial log"},
    {"name": "led_control",      "description": "Control LED / RGB LEDs"},
    {"name": "button_read",      "description": "Read button state and press count"},
    {"name": "wifi_scan",        "description": "Scan nearby Wi-Fi access points"},
    {"name": "gpio_read",        "description": "Read GPIO pin digital level"},
    {"name": "gpio_write",       "description": "Set GPIO pin digital output level"},
    {"name": "firmware_version", "description": "Firmware version, seal, supported tools"},
]

async def health(request: Request) -> JSONResponse:
    return JSONResponse({
        "status": "ok", "version": "0.2.0",
        "timestamp": int(time.time()),
        "devices_online": len(device_mgr._connections),
        "total_calls": call_logger.total,
        "uptime_s": call_logger.uptime_s,
    })

async def mcp_tools(request: Request) -> JSONResponse:
    return JSONResponse({"server": "pclikaPlatform", "tools": TOOL_LIST, "count": len(TOOL_LIST)})


# ── Device WebSocket ───────────────────────────────────────────────────────────
async def device_ws_endpoint(ws: WebSocket):
    await ws.accept()
    try:
        raw = await ws.receive_text()
        msg = json.loads(raw)
        if msg.get("type") != "register":
            await ws.send_text(json.dumps({"type": "error", "error": "first message must be register"}))
            await ws.close(); return
        api_key = msg.get("api_key", "")
        if not auth.validate(api_key):
            await ws.send_text(json.dumps({"type": "error", "error": "invalid api_key"}))
            await ws.close(); return
        board_id = msg.get("board_id", "unknown")
        await ws.send_text(json.dumps({"type": "registered", "board_id": board_id}))
        await device_mgr.register(api_key, ws, board_id)
    except WebSocketDisconnect:
        pass
    except Exception as exc:
        logger.exception("Device WS error: %s", exc)


# ── Admin handlers ─────────────────────────────────────────────────────────────
def _check_admin(request: Request) -> bool:
    return bool(ADMIN_KEY and request.headers.get("X-Admin-Key") == ADMIN_KEY)

async def admin_index(request: Request):
    html = Path(__file__).parent / "admin" / "index.html"
    if html.exists():
        return FileResponse(html)
    return JSONResponse({"error": "admin panel not found"}, status_code=404)

async def admin_status(request: Request) -> JSONResponse:
    if not _check_admin(request):
        return JSONResponse({"error": "forbidden"}, status_code=403)
    return JSONResponse({
        "version": "0.2.0",
        "uptime_s": call_logger.uptime_s,
        "devices_online": len(device_mgr._connections),
        "total_keys": len(auth._keys),
        "total_calls": call_logger.total,
        "timestamp": int(time.time()),
    })

async def admin_devices(request: Request) -> JSONResponse:
    if not _check_admin(request):
        return JSONResponse({"error": "forbidden"}, status_code=403)
    return JSONResponse({"devices": device_mgr.list_devices()})

async def admin_logs(request: Request) -> JSONResponse:
    if not _check_admin(request):
        return JSONResponse({"error": "forbidden"}, status_code=403)
    limit = int(request.query_params.get("limit", 100))
    return JSONResponse({"logs": call_logger.get(limit), "total": call_logger.total})

async def admin_metrics(request: Request) -> JSONResponse:
    if not _check_admin(request):
        return JSONResponse({"error": "forbidden"}, status_code=403)
    return JSONResponse({
        "per_minute": call_logger.per_minute(60),
        "by_tool": call_logger.by_tool(),
    })

async def admin_keys_get(request: Request) -> JSONResponse:
    if not _check_admin(request):
        return JSONResponse({"error": "forbidden"}, status_code=403)
    keys_raw = [
        {"key": k, "key_masked": k[:12] + "…" + k[-4:],
         "label": v.get("label", ""), "created": v.get("created"),
         "demo": v.get("demo", False)}
        for k, v in auth._keys.items()
    ]
    return JSONResponse({"keys": keys_raw})

async def admin_keys_post(request: Request) -> JSONResponse:
    if not _check_admin(request):
        return JSONResponse({"error": "forbidden"}, status_code=403)
    body = await request.json()
    key = auth.create(label=body.get("label", "API Key"))
    return JSONResponse({"key": key}, status_code=201)

async def admin_keys_delete(request: Request) -> JSONResponse:
    if not _check_admin(request):
        return JSONResponse({"error": "forbidden"}, status_code=403)
    key = request.path_params["key"]
    ok = auth.revoke(key)
    return JSONResponse({"ok": ok}, status_code=200 if ok else 404)


# ── Auth middleware ─────────────────────────────────────────────────────────────
PUBLIC_PATHS = {"/health", "/mcp/tools"}

class AuthMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):
        path = request.url.path
        if path in PUBLIC_PATHS or path.startswith("/admin"):
            return await call_next(request)
        api_key = request.headers.get("X-API-Key", "")
        if not auth.validate(api_key):
            return JSONResponse(
                {"error": "unauthorized", "hint": "Add X-API-Key: pck_<32hex> header"},
                status_code=401)
        token = _current_key.set(api_key)
        try:
            return await call_next(request)
        finally:
            _current_key.reset(token)


# ── Routes ─────────────────────────────────────────────────────────────────────
_sse_app  = mcp.sse_app()
_http_app = mcp.streamable_http_app()

routes = [
    Route("/health",               health),
    Route("/mcp/tools",            mcp_tools),
    # Admin
    Route("/admin",                admin_index),
    Route("/admin/",               admin_index),
    Route("/admin/api/status",     admin_status),
    Route("/admin/api/devices",    admin_devices),
    Route("/admin/api/logs",       admin_logs),
    Route("/admin/api/metrics",    admin_metrics),
    Route("/admin/api/keys",       admin_keys_get,    methods=["GET"]),
    Route("/admin/api/keys",       admin_keys_post,   methods=["POST"]),
    Route("/admin/api/keys/{key}", admin_keys_delete, methods=["DELETE"]),
    # Device WebSocket
    WebSocketRoute("/device", device_ws_endpoint),
    # MCP protocol
    Mount("/sse", app=_sse_app),
    Mount("/mcp", app=_http_app),
]

middleware = [
    Middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"]),
    Middleware(AuthMiddleware),
]

app = Starlette(routes=routes, middleware=middleware)

if __name__ == "__main__":
    uvicorn.run("main:app", host="0.0.0.0",
                port=int(os.environ.get("PORT", 8080)), log_level="info")
