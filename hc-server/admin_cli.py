#!/usr/bin/env python3
"""
pclika-admin — CLI 管理工具
用法:
  python admin_cli.py status
  python admin_cli.py keys list
  python admin_cli.py keys create "My Board"
  python admin_cli.py keys revoke pck_xxx
  python admin_cli.py devices
  python admin_cli.py logs [--limit N]
  python admin_cli.py tail         ← 实时滚动日志

环境变量:
  PCLIKA_SERVER   服务器地址（默认 https://hc.pclika.com）
  PCLIKA_ADMIN_KEY  Admin Key
"""
import argparse
import json
import os
import sys
import time
import urllib.request
import urllib.error

SERVER = os.environ.get("PCLIKA_SERVER", "https://hc.pclika.com").rstrip("/")
ADMIN_KEY = os.environ.get("PCLIKA_ADMIN_KEY", "")

# ── Colors ────────────────────────────────────────────────────────────────────
CYAN  = "\033[96m"; GREEN = "\033[92m"; YELLOW = "\033[93m"
RED   = "\033[91m"; DIM   = "\033[2m";  BOLD   = "\033[1m"; RESET = "\033[0m"

def c(color, text): return color + str(text) + RESET

# ── HTTP ──────────────────────────────────────────────────────────────────────
def req(method, path, body=None, key=None):
    url = SERVER + path
    data = json.dumps(body).encode() if body else None
    headers = {
        "X-Admin-Key": key or ADMIN_KEY,
        "Content-Type": "application/json",
    }
    r = urllib.request.Request(url, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(r, timeout=10) as resp:
            return json.load(resp)
    except urllib.error.HTTPError as e:
        return json.load(e)
    except Exception as exc:
        print(c(RED, f"Error: {exc}"))
        sys.exit(1)

def ensure_key():
    global ADMIN_KEY
    if not ADMIN_KEY:
        ADMIN_KEY = input("Admin Key: ").strip()
    if not ADMIN_KEY:
        print(c(RED, "需要 PCLIKA_ADMIN_KEY")); sys.exit(1)

# ── Commands ──────────────────────────────────────────────────────────────────
def cmd_status():
    s = req("GET", "/admin/api/status")
    print(c(BOLD, "\n  Pclika MCP Platform  ") + c(GREEN, "● online\n"))
    print(f"  版本       {c(CYAN, s.get('version','?'))}")
    up = s.get('uptime_s', 0)
    fmt = f"{up//3600}h {(up%3600)//60}m {up%60}s"
    print(f"  运行时间   {c(CYAN, fmt)}")
    print(f"  在线设备   {c(GREEN if s.get('devices_online') else DIM, s.get('devices_online',0))}")
    print(f"  API Keys   {c(CYAN, s.get('total_keys',0))}")
    print(f"  工具调用   {c(CYAN, s.get('total_calls',0))}\n")


def cmd_keys_list():
    data = req("GET", "/admin/api/keys")
    keys = data.get("keys", [])
    print(c(BOLD, f"\n  API Keys ({len(keys)})\n"))
    print(f"  {'Key':40} {'标签':20} {'创建时间':22}")
    print("  " + "─" * 85)
    for k in keys:
        ts = time.strftime('%Y-%m-%d %H:%M', time.localtime(k.get('created', 0))) if k.get('created') else '—'
        demo = c(YELLOW, " [DEMO]") if k.get("demo") else ""
        print(f"  {c(CYAN, k['key_masked']):49} {k.get('label',''):20} {DIM+ts+RESET:30}{demo}")
    print()


def cmd_keys_create(label):
    data = req("POST", "/admin/api/keys", {"label": label})
    key = data.get("key", "")
    print(c(GREEN, "\n  ✓ Key 已创建\n"))
    print(f"  {c(CYAN, key)}\n")
    print(c(DIM, "  Claude Code 接入命令："))
    print(f'  claude mcp add pclikaPlatform --transport sse https://hc.pclika.com/sse --header "X-API-Key: {key}"\n')


def cmd_keys_revoke(key):
    confirm = input(f"确定吊销 {key[:20]}…? [y/N] ").strip().lower()
    if confirm != "y":
        print("取消"); return
    data = req("DELETE", f"/admin/api/keys/{key}")
    if data.get("ok"):
        print(c(GREEN, "✓ 已吊销"))
    else:
        print(c(RED, "✗ 未找到该 Key"))


def cmd_devices():
    data = req("GET", "/admin/api/devices")
    devices = data.get("devices", [])
    print(c(BOLD, f"\n  在线设备 ({len(devices)})\n"))
    if not devices:
        print(c(DIM, "  无在线设备。设备通过 wss://hc.pclika.com/device 注册。\n"))
        return
    print(f"  {'Board ID':20} {'Key':15} {'连接时间':22} {'调用次数':10}")
    print("  " + "─" * 70)
    for d in devices:
        ts = time.strftime('%H:%M:%S', time.localtime(d.get('connected_at', 0)))
        print(f"  {c(CYAN, d.get('board_id','?')):29} {d.get('key','?'):15} {ts:22} {d.get('call_count',0)}")
    print()


def cmd_logs(limit=50):
    data = req("GET", f"/admin/api/logs?limit={limit}")
    logs = data.get("logs", [])
    total = data.get("total", 0)
    print(c(BOLD, f"\n  调用日志（最近 {len(logs)} 条，共 {total} 条）\n"))
    print(f"  {'时间':10} {'Key':15} {'工具':22} {'耗时':8} {'状态'}")
    print("  " + "─" * 65)
    for e in logs:
        ts = time.strftime('%H:%M:%S', time.localtime(e['ts']))
        st = c(GREEN, "OK ") if e["ok"] else c(RED, "ERR")
        demo = c(YELLOW, " demo") if e.get("demo") else ""
        print(f"  {DIM+ts+RESET:19} {e['key']:15} {e['tool']:22} {str(e['duration_ms'])+'ms':9}{st}{demo}")
    print()


def cmd_tail():
    print(c(CYAN, f"\n  实时日志 — {SERVER}\n") + c(DIM, "  Ctrl+C 退出\n"))
    seen = set()
    while True:
        try:
            data = req("GET", "/admin/api/logs?limit=20")
            for e in reversed(data.get("logs", [])):
                uid = f"{e['ts']}-{e['key']}-{e['tool']}"
                if uid not in seen:
                    seen.add(uid)
                    ts = time.strftime('%H:%M:%S', time.localtime(e['ts']))
                    st = c(GREEN, "OK") if e["ok"] else c(RED, "ERR")
                    demo = c(YELLOW, " [demo]") if e.get("demo") else ""
                    print(f"  {DIM+ts+RESET}  {c(CYAN, e['key']):24}  {e['tool']:22}  {str(e['duration_ms'])+'ms':8} {st}{demo}")
            time.sleep(2)
        except KeyboardInterrupt:
            print("\n"); break


# ── Main ──────────────────────────────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(prog="admin_cli", description="Pclika MCP Admin CLI")
    sub = parser.add_subparsers(dest="cmd")

    sub.add_parser("status", help="服务器状态")

    k = sub.add_parser("keys", help="API Key 管理")
    ks = k.add_subparsers(dest="keys_cmd")
    ks.add_parser("list")
    kc = ks.add_parser("create"); kc.add_argument("label", nargs="?", default="API Key")
    kr = ks.add_parser("revoke"); kr.add_argument("key")

    sub.add_parser("devices", help="在线设备列表")
    lo = sub.add_parser("logs", help="调用日志")
    lo.add_argument("--limit", type=int, default=50)
    sub.add_parser("tail", help="实时滚动日志")

    args = parser.parse_args()
    ensure_key()

    if args.cmd == "status":      cmd_status()
    elif args.cmd == "keys":
        if args.keys_cmd == "list":   cmd_keys_list()
        elif args.keys_cmd == "create": cmd_keys_create(args.label)
        elif args.keys_cmd == "revoke": cmd_keys_revoke(args.key)
        else: k.print_help()
    elif args.cmd == "devices":   cmd_devices()
    elif args.cmd == "logs":      cmd_logs(args.limit)
    elif args.cmd == "tail":      cmd_tail()
    else: parser.print_help()

if __name__ == "__main__":
    main()
