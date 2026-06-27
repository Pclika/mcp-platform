# uart-echo — UART Loopback Example

Receives bytes on `uart_rx` and immediately echoes them back on `uart_tx`.
Green LED blinks on each received byte; red LED latches on framing error;
blue LED heartbeats at ~1 Hz.

## Hardware

- Board: iCEBreaker (iCE40UP5K SG48)
- Clock: 12 MHz onboard oscillator
- Baud: 115200 8N1 via USB-UART bridge

## Build

```bash
# Lint → synthesize → implement → pack → flash
make flash

# Simulate only
make sim
```

## Test

Connect with any serial terminal at 115200 baud:

```bash
screen /dev/ttyUSB1 115200
# or
python3 -c "
import serial, time
s = serial.Serial('/dev/ttyUSB1', 115200, timeout=1)
s.write(b'Hello\r\n')
time.sleep(0.1)
print(repr(s.read(7)))
s.close()
"
```

## File layout

```
uart-echo/
├── rtl/uart_echo_top.v        # top-level (uses pclika_uart_rx + pclika_uart_tx)
├── tb/tb_uart_echo_top.v      # simulation testbench
├── constraints/ice40up5k.pcf  # pin assignments
└── Makefile
```

## Seal

`PCK-MMXXVI-9198580D`
