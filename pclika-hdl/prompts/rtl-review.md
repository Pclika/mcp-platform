# Prompt: RTL Review

Use when asking Claude or Codex to review RTL for synthesizability and correctness.

---

## Context block

```
Please review the following Verilog RTL for the iCE40UP5K target.

Check for:

SYNTHESIZABILITY
- No $display, $monitor, $dumpfile, $finish (simulation-only constructs)
- No #delay (timing control)
- No initial blocks outside testbenches
- All latches must be intentional (flag if unintentional)
- All case statements should be full or have a default

RESET STYLE
- Prefer synchronous reset (posedge clk) for iCE40
- Flag any async reset (negedge rst_n in always @) — needs justification

PARAMETERS
- Parameters should be UPPER_SNAKE_CASE
- Magic numbers should be parameterized

PORTABILITY
- No iCE40-specific primitives unless justified (SB_PLL40, SB_IO, etc.)
- Flag any vendor-specific constructs

NAMING
- Modules: pclika_<function>
- Ports and signals: lower_snake_case
- Registers: _r suffix
- Wires: _w suffix

RESOURCE ESTIMATE
- Rough estimate: how many LUT4 would this use?
- Does it fit in iCE40UP5K's 5280 LUT4 budget?

Output format:
1. PASS / WARN / FAIL — overall verdict
2. List of issues by category
3. Corrected code if any changes are needed
```
