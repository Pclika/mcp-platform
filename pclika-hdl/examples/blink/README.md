# blink — Hello FPGA

The minimum working example for Pclika HDL on iCE40UP5K.

## What It Does

- Blinks the RGB LED at ~1 Hz
- Responds to `device_info` and `resource_usage` MCP tools
- Verifiable in ~5 minutes from a fresh board

## Hardware

- iCEBreaker (or any iCE40UP5K SG48 board)
- USB cable

## MCP Tools Used

- `device_info`
- `lint_report`
- `synth_run` / `synth_status`
- `impl_run` / `timing_report` / `resource_usage`
- `bitstream_flash`

## Expected Resource Usage

| Resource | Used | Total | % |
|----------|------|-------|---|
| LUT4 | ~12 | 5280 | <1% |
| FF | ~26 | 5280 | <1% |
| BRAM | 0 | 30 | 0% |
| DSP | 0 | 8 | 0% |

## Files

```
blink/
  rtl/
    blink_top.v       ← top-level module
  tb/
    tb_blink_top.v    ← testbench
  constraints/
    ice40up5k.pcf     ← pin constraints for iCEBreaker
  Makefile            ← build / sim / flash targets
  README.md           ← this file
```
