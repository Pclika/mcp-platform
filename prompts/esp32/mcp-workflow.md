# MCP Workflow Prompt

Paste this when you want Claude or Codex to drive a multi-step hardware workflow via MCP tool calls.

---

```
You are connected to a Pclika MCP Platform device via the pclika-bridge MCP server (server name: pclikaPlatform).

You have access to 23 MCP tools. Always start by calling `device_info()` to confirm the connection and check which capabilities are registered.

## Workflow request

[Describe what you want to accomplish. Examples below — delete all but one.]

### Option A — Environment monitor
Continuously read temperature, humidity, and light levels every 5 seconds. Display the current reading on the OLED. Blink the LED red when temperature exceeds 30 °C.

### Option B — Servo calibration
Sweep servo channel 0 from 0° to 180° in 10° increments, pausing 500 ms at each position. Report the actual pulse width at each step.

### Option C — Wi-Fi site survey
Scan for all available networks. Sort by signal strength. Display the top 3 SSIDs and their RSSI on the OLED display. Output the full list as a formatted table.

### Option D — GPIO loopback test
Set GPIO 5 high, read GPIO 6, then set GPIO 5 low, read GPIO 6 again. Confirm the signal is propagated correctly. Repeat 10 times and report any failures.

---

## Rules for this session

1. Call `device_info()` first. If `seal` ≠ `PCK-MMXXVI-C4A32096`, halt and report a seal mismatch.
2. Check `capabilities[]` before calling extension tools. If the required capability is absent, report it rather than calling a tool that will fail.
3. For sensor reads, always include `timestamp_ms` in any logs you produce.
4. If a tool call fails with a JSON-RPC error, parse the `error.code` and `error.message` and suggest a corrective action before retrying.
5. Do not loop more than 20 times without pausing to summarize results to the user.
6. Keep all intermediate results in a structured format (dict / JSON) so I can inspect them.

## Output format

After each major step, output:
- The tool called and its arguments
- The key result fields
- Any anomalies or errors
- Next planned action
```
