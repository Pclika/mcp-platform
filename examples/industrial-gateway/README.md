# industrial-gateway

MCP example for the Pclika Platform.  
Demonstrates industrial I/O: Modbus RTU over RS-485, relay control, ADC, and digital inputs.

**Seal:** `PCK-MMXXVI-C4A32096`

## What it does

| MCP Tool | Hardware | Description |
|----------|----------|-------------|
| `modbus_read`     | RS-485 transceiver (UART2) | Read holding/input registers and coils from Modbus slaves |
| `modbus_write`    | RS-485 transceiver (UART2) | Write registers or coils to Modbus slaves |
| `relay_set`       | 8-channel relay module      | Set, toggle, or pulse relay outputs |
| `digital_io_read` | GPIO bank (pins 32–39)      | Read 6 digital input channels simultaneously |
| `adc_read_multi`  | Built-in ADC1               | Read up to 4 analog channels with voltage conversion |

## Hardware

- ESP32 (not S3 — legacy ADC API)
- RS-485 transceiver (MAX485, SP3485, or similar) on UART2 TX=17/RX=16/DE=5
- 8-channel relay module on GPIO 18/19/21/22/23/25/26/27
- Digital inputs on GPIO 32/33/34/35/36/39 (pull-down enabled)
- Analog inputs on ADC1 CH0/3/6/7

## Default Pin Map

| Signal | GPIO |
|--------|------|
| RS-485 TX | 17 |
| RS-485 RX | 16 |
| RS-485 DE (enable) | 5 |
| Relay 0–7 | 18, 19, 21, 22, 23, 25, 26, 27 |
| Digital In 0–5 | 32, 33, 34, 35, 36, 39 |
| ADC Ch 0–3 | GPIO32, GPIO39, GPIO34, GPIO35 |

## Build & Flash

```bash
idf.py set-target esp32
idf.py build flash monitor
```

## Connect with AI

```bash
claude mcp add pclika-bridge -- pclika-bridge --port /dev/ttyUSB0

# Read Modbus slave 1, holding registers 0–4
modbus_read(slave_id=1, function="holding_registers", address=0, count=5)

# Turn relay 0 on, pulse for 500 ms
relay_set(channel=0, state="on", pulse_ms=500)

# Read all 4 ADC channels
adc_read_multi(channels=[0,1,2,3])
```

## Python Demo

```bash
pip install ./bridge/mcp-server
python demo.py --port /dev/ttyUSB0 --skip-modbus   # skip if no RS-485 device
python demo.py --port /dev/ttyUSB0 --slave 1 --loop 5
```

## License

Apache-2.0 — Copyright 2026 [Pclika](https://pclika.com)
