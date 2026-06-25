# Claude Code MCP Setup Template

Use the future Pclika bridge with a short, stable server name:

- `pclikaPlatform`

## Local Scope

Run from the project directory once the bridge command is ready:

```bash
claude mcp add pclikaPlatform -- node bridge/mcp-server/index.js
```

## User Scope

If you want the server available across projects:

```bash
claude mcp add --scope user pclikaPlatform -- node /absolute/path/to/bridge/mcp-server/index.js
```

## HTTP Variant

If the platform later exposes a remote MCP server:

```bash
claude mcp add --transport http pclikaPlatform https://REPLACE-WITH-PCLIKA-MCP-ENDPOINT
```

## Verify

```bash
claude mcp list
```

In Claude Code, use `/mcp` to confirm the server is connected.
