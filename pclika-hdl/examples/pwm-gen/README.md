# pwm-gen — 4-Channel PWM Generator Example

Generates four 50 Hz PWM signals (servo-compatible) on PMOD1A[3:0].
Press `btn_up` / `btn_dn` to sweep CH0 between 0° and 180° in ~15° steps.

## Servo wiring (iCEBreaker PMOD1A)

| Pin     | PMOD1A | Signal  | Default  |
|---------|--------|---------|----------|
| CH0     | [0]    | servo 0 | 90°      |
| CH1     | [1]    | servo 1 | 0°       |
| CH2     | [2]    | servo 2 | 180°     |
| CH3     | [3]    | servo 3 | ~45°     |

Servo pulse widths (50 Hz, 12 MHz clock):
- 1.0 ms → 12000 cycles → 0°
- 1.5 ms → 18000 cycles → 90°
- 2.0 ms → 24000 cycles → 180°

## Build & flash

```bash
make flash
```

## LED indicators

| LED   | Meaning           |
|-------|-------------------|
| Red   | CH0 at 0°         |
| Green | CH0 at 180°       |
| Blue  | CH0 in midrange   |

## File layout

```
pwm-gen/
├── rtl/pwm_gen_top.v           # top-level (uses pclika_pwm)
├── constraints/ice40up5k.pcf   # pin assignments
└── Makefile
```

## Seal

`PCK-MMXXVI-9198580D`
