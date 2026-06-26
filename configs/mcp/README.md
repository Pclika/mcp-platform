# Pclika MCP Client Configurations

Two connection modes — pick one, all five clients work the same way.

## Quick Start

```bash
pip install pclika-bridge
pclika-setup          # auto-detect installed clients + write configs
```

---

## Mode A — STDIO (default)

Each AI client spawns its own `pclika-bridge` subprocess.
No extra process needed. Works out of the box.

| Client     | Config file             | Install location |
|------------|-------------------------|------------------|
| Claude Code | `claude-code.commands.md` | Run the `claude mcp add` commands once |
| Cursor      | `cursor.mcp.json`       | `~/.cursor/mcp.json` |
| VS Code     | `vscode.mcp.json`       | `.vscode/mcp.json` or `settings.json` |
| Codex       | `codex.config.toml`     | `~/.config/codex/config.toml` |
| OpenCode    | `opencode.jsonc`        | `~/.config/opencode/config.json` |

Auto-install for all detected clients:
```bash
pclika-setup
```

---

## Mode B — SSE / Adaptive (multi-client)

Run **