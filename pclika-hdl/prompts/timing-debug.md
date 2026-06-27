# Prompt: Timing Debug

Use when timing_report() shows negative slack or violated paths.

---

## Context block

```
I have a timing violation in my iCE40UP5K design.

MCP server: pclikaHDL
Call: timing_report(verbose=true) to get the full critical path.

iCE40UP5K timing reference:
- LUT4 propagation delay: ~0.3–0.6 ns
- FF setup time: ~0.2 ns
- Routing delays: 0.1–2.0 ns depending on distance
- Max reliable frequency for random logic: 40–60 MHz
- Max reliable frequency for pipelined designs: 80–100 MHz

Common causes of timing failure on iCE40UP5K:
1. Long carry chains — use DSP blocks instead
2. Wide muxes — break into smaller, pipelined muxes
3. Long combinatorial paths — add pipeline registers
4. Unregistered outputs — register before the output FF
5. Single PLL — can't generate multiple clocks without careful constraint

Fixes I want you to apply:
- Add pipeline register(s) in the critical path
- Keep register names with _r suffix
- Add a comment marking inserted pipeline stages with -- PIPELINE --
- Re-run: lint_report → synth_run → impl_run → timing_report
```

---

## Timing check after fix

```
After each RTL change:
1. lint_report()             — no new errors
2. synth_run(top_module=...) — check cell count didn't explode
3. impl_run(freq_mhz=12)    — let nextpnr re-route
4. timing_report()           — verify worst_slack ≥ 0
```
