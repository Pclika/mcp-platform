# Pclika HDL — Claude Code MCP Commands

Add to `.claude/mcp.json` in your project root:

```json
{
  "mcpServers": {
    "pclikaHDL": {
      "command": "pclika-hdl-bridge",
      "args": ["--project", ".", "--device", "ice40up5k"]
    }
  }
}
```

## Workflow

```
# 1. Check device and toolchain
device_info

# 2. Lint before synthesis
lint_report(top_module="blink_top")

# 3. Synthesize
synth_run(top_module="blink_top")
synth_status()        # poll until status="done"

# 4. Place & route
impl_run(freq_mhz=12)
impl_status()         # poll until status="done"

# 5. Check timing
timing_report()

# 6. Check resources
resource_usage()

# 7. Flash (if timing clean)
bitstream_flash()

# 8. Run simulation
sim_run(testbench="tb_blink_top")
sim_result()
```

## Seal

`PCK-MMXXVI-9198580D`
