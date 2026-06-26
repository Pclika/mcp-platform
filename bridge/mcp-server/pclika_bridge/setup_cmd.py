"""
pclika-setup — auto-install MCP configs for all detected AI clients.

Detects: Claude Code, Cursor, VS Code, OpenCode, Codex
Writes the correct config file to each client's expected location.
Supports both STDIO mode and SSE (adaptive / multi-client) mode.
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path
from textwrap import dedent

SSE_URL = "http://localhost:3141/sse"
BRIDGE_CMD = "pclika-bridge"
HDL_CMD = "pclika-hdl-bridge"

# ── Client detection ───────────────────────────────────────────────────────────

def _home() -> Path:
    return Path.home()

def _win() -> bool:
    return platform.system() == "Windows"

def _mac() -> bool:
    return platform.system() == "Darwin"


def detect_clients() -> dict[str, Path | None]:
    """
    Returns dict of client_name → config_path (None if not detected).
    Config path is where we will WRITE the config.
    """
    h = _home()
    results: dict[str, Path | None] = {}

    # ── Claude Code ──────────────────────────────────────────────────────────
    # claude binary exists + config at ~/.claude/mcp.json (or similar)
    claude_bin = shutil.which("claude")
    if claude_bin:
        if _win():
            cfg = h / "AppData" / "Roaming" / "Claude" / "claude_desktop_config.json"
        elif _mac():
            cfg = h / "Library" / "Application Support" / "Claude" / "claude_desktop_config.json"
        else:
            cfg = h / ".config" / "Claude" / "claude_desktop_config.json"
        results["claude"] = cfg
    else:
        results["claude"] = None

    # ── Cursor ──────────────────────────────────────────────────────────────
    if _win():
        cursor_cfg = h / "AppData" / "Roaming" / "Cursor" / "User" / "globalStorage" / "mcp.json"
    elif _mac():
        cursor_cfg = h / "Library" / "Application Support" / "Cursor" / "User" / "globalStorage" / "mcp.json"
    else:
        cursor_cfg = h / ".config" / "Cursor" / "User" / "globalStorage" / "mcp.json"
    results["cursor"] = cursor_cfg if cursor_cfg.parent.parent.exists() else None

    # ── VS Code ─────────────────────────────────────────────────────────────
    if _win():
        vscode_cfg = h / "AppData" / "Roaming" / "Code" / "User" / "settings.json"
    elif _mac():
        vscode_cfg = h / "Library" / "Application Support" / "Code" / "User" / "settings.json"
    else:
        vscode_cfg = h / ".config" / "Code" / "User" / "settings.json"
    results["vscode"] = vscode_cfg if vscode_cfg.parent.exists() else None

    # ── OpenCode ────────────────────────────────────────────────────────────
    opencode_bin = shutil.which("opencode")
    if opencode_bin:
        cfg = h / ".config" / "opencode" / "config.json"
        results["opencode"] = cfg
    else:
        results["opencode"] = None

    # ── Codex ───────────────────────────────────────────────────────────────
    codex_bin = shutil.which("codex")
    if codex_bin:
        if _win():
            cfg = h / "AppData" / "Roaming" / "codex" / "config.toml"
        else:
            cfg = h / ".config" / "codex" / "config.toml"
        results["codex"] = cfg
    else:
        results["codex"] = None

    return results


# ── Config writers ─────────────────────────────────────────────────────────────

def _write_json(path: Path, data: dict):
    path.parent.mkdir(parents=True, exist_ok=True)
    # Merge into existing file if present (don't overwrite unrelated settings)
    existing = {}
    if path.exists():
        try:
            existing = json.loads(path.read_text(encoding="utf-8"))
        except Exception:
            pass
    _deep_merge(existing, data)
    path.write_text(
        json.dumps(existing, indent=2, ensure_ascii=False),
        encoding="utf-8"
    )


def _deep_merge(base: dict, override: dict):
    for k, v in override.items():
        if k in base and isinstance(base[k], dict) and isinstance(v, dict):
            _deep_merge(base[k], v)
        else:
            base[k] = v


def install_claude(path: Path, sse: bool):
    """claude_desktop_config.json  mcpServers block"""
    servers: dict = {
        "pclikaPlatform": {
            "command": BRIDGE_CMD,
            "args": ["--port", "auto", "--baud", "115200"],
        },
        "pclikaHDL": {
            "command": HDL_CMD,
            "args": ["--device", "ice40up5k", "--package", "sg48", "--freq", "12"],
        },
    }
    if sse:
        servers["pclikaPlatform"] = {"url": SSE_URL}

    _write_json(path, {"mcpServers": servers})
    print(f"  [claude]   ✓  {path}")
    if not sse:
        print("             Also run: claude mcp add pclikaPlatform -- pclika-bridge --port auto")


def install_cursor(path: Path, sse: bool):
    """~/.cursor/mcp.json  mcpServers block"""
    if sse:
        entry: dict = {"url": SSE_URL}
    else:
        entry = {
            "command": BRIDGE_CMD,
            "args": ["--port", "auto", "--baud", "115200"],
            "env": {},
        }
    _write_json(path, {
        "mcpServers": {
            "pclikaPlatform": entry,
            "pclikaHDL": {
                "command": HDL_CMD,
                "args": ["--device", "ice40up5k", "--package", "sg48", "--freq", "12"],
                "env": {},
            },
        }
    })
    print(f"  [cursor]   ✓  {path}")


def install_vscode(path: Path, sse: bool):
    """settings.json  mcp.servers block"""
    if sse:
        entry: dict = {"type": "sse", "url": SSE_URL}
    else:
        entry = {
            "type": "stdio",
            "command": BRIDGE_CMD,
            "args": ["--port", "auto", "--baud", "115200"],
            "env": {},
        }
    _write_json(path, {
        "mcp": {
            "servers": {
                "pclikaPlatform": entry,
                "pclikaHDL": {
                    "type": "stdio",
                    "command": HDL_CMD,
                    "args": ["--device", "ice40up5k", "--package", "sg48", "--freq", "12"],
                    "env": {},
                },
            }
        }
    })
    print(f"  [vscode]   ✓  {path}")


def install_opencode(path: Path, sse: bool):
    """~/.config/opencode/config.json"""
    if sse:
        entry: dict = {"type": "remote", "url": SSE_URL, "enabled": True}
    else:
        entry = {
            "type": "local",
            "command": BRIDGE_CMD,
            "args": ["--port", "auto"],
            "enabled": True,
        }
    _write_json(path, {
        "mcp": {
            "pclikaPlatform": entry,
            "pclikaHDL": {
                "type": "local",
                "command": HDL_CMD,
                "args": ["--device", "ice40up5k"],
                "enabled": True,
            },
        }
    })
    print(f"  [opencode] ✓  {path}")


def install_codex(path: Path, sse: bool):
    """~/.config/codex/config.toml"""
    path.parent.mkdir(parents=True, exist_ok=True)
    if sse:
        block = dedent(f"""\
            [mcp_servers.pclikaPlatform]
            url = "{SSE_URL}"
            enabled = true
            startup_timeout_sec = 5
            tool_timeout_sec = 30

            [mcp_servers.pclikaHDL]
            command = "{HDL_CMD}"
            args = ["--device", "ice40up5k", "--package", "sg48", "--freq", "12"]
            enabled = true
            startup_timeout_sec = 15
            tool_timeout_sec = 120
        """)
    else:
        block = dedent(f"""\
            [mcp_servers.pclikaPlatform]
            command = "{BRIDGE_CMD}"
            args = ["--port", "auto", "--baud", "115200"]
            enabled = true
            startup_timeout_sec = 10
            tool_timeout_sec = 30

            [mcp_servers.pclikaHDL]
            command = "{HDL_CMD}"
            args = ["--device", "ice40up5k", "--package", "sg48", "--freq", "12"]
            enabled = true
            startup_timeout_sec = 15
            tool_timeout_sec = 120
        """)
    # Append or replace the pclika block in an existing toml
    existing = ""
    if path.exists():
        existing = path.read_text(encoding="utf-8")
        # Remove any existing pclika blocks
        import re
        existing = re.sub(
            r"\[mcp_servers\.pclika\w+\][^\[]*", "", existing, flags=re.DOTALL
        ).strip()
    path.write_text((existing + "\n\n" + block).strip() + "\n", encoding="utf-8")
    print(f"  [codex]    ✓  {path}")


INSTALLERS = {
    "claude":   install_claude,
    "cursor":   install_cursor,
    "vscode":   install_vscode,
    "opencode": install_opencode,
    "codex":    install_codex,
}


# ── CLI ────────────────────────────────────────────────────────────────────────

def setup_main():
    parser = argparse.ArgumentParser(
        prog="pclika-setup",
        description="Auto-install Pclika MCP configs for all detected AI clients.",
    )
    parser.add_argument(
        "--client", "-c",
        choices=list(INSTALLERS),
        help="Install for a specific client only",
    )
    parser.add_argument(
        "--sse", action="store_true",
        help=(
            "Use SSE (adaptive) mode: all clients connect to "
            f"{SSE_URL} — run 'pclika-bridge --serve' first"
        ),
    )
    parser.add_argument(
        "--list", "-l", action="store_true",
        help="Show detected clients and exit",
    )
    args = parser.parse_args()

    clients = detect_clients()

    if args.list:
        print("\nDetected clients:")
        for name, path in clients.items():
            status = f"✓  {path}" if path else "✗  not found"
            print(f"  {name:<12} {status}")
        print()
        return

    mode = "SSE (adaptive)" if args.sse else "STDIO (per-client process)"
    print(f"\npclika-setup  —  mode: {mode}\n")

    targets = {args.client: clients[args.client]} if args.client else clients

    installed = 0
    for name, path in targets.items():
        if path is None:
            print(f"  [{name:<9}] —  not detected, skipping")
            continue
        try:
            INSTALLERS[name](path, sse=args.sse)
            installed += 1
        except Exception as exc:
            print(f"  [{name:<9}] ✗  ERROR: {exc}")

    print(f"\n{installed} client(s) configured.")
    if args.sse:
        print(f"\nStart the shared bridge with:")
        print(f"  pclika-bridge --serve")
        print(f"  # then open any client — they all share one connection\n")
    else:
        print(f"\nEach client will launch its own bridge process automatically.\n")


if __name__ == "__main__":
    setup_main()
