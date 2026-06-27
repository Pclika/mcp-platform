# Prompt: Synthesis Workflow

Use this prompt context when working on RTL synthesis with Claude or Codex.

---

## Context block (paste into your AI tool)

```
I'm working on an FPGA design using the Pclika HDL Platform.

Target device: iCE40UP5K (SG48 package), 12 MHz system clock
Toolchain: Yosys (synthesis) + nextpnr-ice40 (P&R) + Verilator (sim/lint)
MCP server: pclikaHDL (connected via pclika-hdl-bridge)

Standard workflow order:
1. lint_report      — fix all errors and critical warnings first
2. synth_run        — synthesize with Yosys
3. synth_status     — verify: no errors, check cell count
4. impl_run         — place & route with nextpnr
5. impl_status      — verify: routing_success=true
6. timing_report    — verify: worst_slack ≥ 0, no violated paths
7. resource_usage   — verify: LUT usage < 80%
8. bitstream_flash  — only after timing is clean

RTL rules:
- All RTL in hdl/rtl/ must be synthesizable (no $display, no #delay)
- Simulation-only code goes in hdl/tb/ only
- Modules: pclika_<function>  (e.g. pclika_uart_rx)
- Testbenches: tb_<module>    (e.g. tb_pclika_uart_rx)
- Parameters: UPPER_SNAKE_CASE
- Ports and signals: lower_snake_case
- Registers: _r suffix   (e.g. cnt_r)
- Wires: _w suffix       (e.g. data_w)
```

---

## Troubleshooting prompts

**If synth_status shows errors:**
```
synth_status() returned errors. Show me the error list and fix each one.
The source file is hdl/rtl/<module>.v.
```

**If impl_status shows routing failed:**
```
impl_status() shows routing_success=false.
Try impl_run(seed=42) and check if a different seed resolves routing.
If still failing, suggest which signals could be relaxed or pipelined.
```

**If timing_report shows negative slack:**
```
timing_report() shows worst_slack < 0 on clock domain 'clk'.
Identify the critical path from the verbose timing log.
Suggest: register insertion, logic restructuring, or frequency relaxation.
```

**If resource_usage shows >80% LUT:**
```
resource_usage() shows high LUT utilization.
Review hdl/rtl/ and identify modules that can be shared, parameterized,
or replaced with IP blocks (BRAM-based, DSP-based).
```
