# Pclika HDL — Hardware List & IP Library Reference

> Every hardware component listed here has at least one corresponding IP module in
> `hdl/rtl/`. The mapping table at the bottom cross-references hardware → IP → example.

---

## 1. Development Boards

### iCEBreaker v1.0 ★ Primary Target

| Field | Value |
|-------|-------|
| FPGA | Lattice iCE40UP5K (SG48) |
| Clock | 12 MHz onboard oscillator |
| Flash | 128 Mbit SPI NOR (W25Q128JV) |
| USB | FT2232H — dual-channel USB-UART/JTAG bridge |
| RGB LED | 1× RGB (active-low, pins 41/40/39) |
| Buttons | 3× push button (active-low) |
| PMOD | 2× PMOD 1A/1B (top) + 1× PMOD 2 (left, 8-pin) |
| Vendor | 1BitSquared |
| Where to buy | https://1bitsquared.com/products/icebreaker |

**IP modules used on this board:**

| Interface | IP Module | Pin(s) |
|-----------|-----------|--------|
| UART (via FT2232H) | `pclika_uart_rx` + `pclika_uart_tx` | RX=6, TX=9 |
| SPI Flash | `pclika_spi_master` | SCK=15, MOSI=14, MISO=17, CS=16 |
| RGB LED | GPIO / `pclika_pwm` | R=41, G=40, B=39 |
| PWM output (PMOD) | `pclika_pwm` | PMOD1A[0..3] = 4,2,47,45 |

---

### Upduino v3.1

| Field | Value |
|-------|-------|
| FPGA | Lattice iCE40UP5K (SG48) — same device as iCEBreaker |
| Clock | No onboard oscillator (must drive from SB_HFOSC internal or external) |
| Flash | 32 Mbit SPI NOR |
| USB | CH340 USB-Serial bridge |
| RGB LED | 1× RGB onboard |
| Form factor | Breadboard-friendly (0.1″ pitch) |
| Vendor | tinyvision.ai |

**Compatibility:** All Pclika HDL RTL is device-level compatible. Constraint files differ (pin numbers vary); use `constraints/upduino_v3.pcf` when targeting Upduino.

---

### TinyFPGA BX

| Field | Value |
|-------|-------|
| FPGA | Lattice iCE40LP8K (CM81) — different device |
| Clock | 16 MHz |
| Flash | 4 Mbit |
| USB | Direct USB (bit-banged via RTL) |
| Status | Partial support — Pclika IP works but boot flow differs |

---

## 2. On-board Peripherals (iCEBreaker)

### FT2232H — USB-UART/JTAG Bridge

Built into iCEBreaker. Channel A = JTAG (for debug); Channel B = UART (primary data path for `pclika_uart_*`).

**Associated IP:** `pclika_uart_rx`, `pclika_uart_tx`

Default baud: 115200. Max reliable baud on iCE40UP5K @ 12 MHz: **921600** (BAUD_DIV = 13, error < 0.5%).

---

### W25Q128JV — SPI NOR Flash

128 Mbit (16 MB) onboard flash. Stores the FPGA bitstream (first 2 MB) and optionally user data (remaining 14 MB).

**Associated IP:** `pclika_spi_master` (CPOL=0, CPHA=0, up to 50 MHz)

Common use: store lookup tables, audio samples, font bitmaps in upper flash pages.

---

### RGB LED (active-low)

Three individual LEDs driven directly from FPGA I/O. No current-limiting resistor needed — iCE40 I/O has internal pull strength control.

**Associated IP:** GPIO direct assign, or `pclika_pwm` for brightness control.

```verilog
assign led_r = 1'b1;   // off (active-low)
assign led_g = ~state; // blinks
assign led_b = 1'b1;   // off
```

---

### Push Buttons (3×)

Active-low with internal pull-up. Require debounce in RTL (10 ms, ~120000 cycles @ 12 MHz).

**Associated IP:** Debounce logic is inline in `pwm_gen_top.v` — copy the 2-FF sync + counter pattern.

---

## 3. PMOD Expansion Modules

PMOD is a 12-pin standard (6 signal + VCC + GND × 2). iCEBreaker has three PMOD connectors.

### PMOD-UART (Digilent PmodUSBUART)

Adds a second USB-UART channel via CP2102. Useful for debugging while the main FT2232H channel carries application data.

**Associated IP:** `pclika_uart_rx` + `pclika_uart_tx`  
**Pins (PMOD1A):** TX=pmod1a[0], RX=pmod1a[1]

---

### PMOD-SPI Sensors

Any SPI-mode sensor on a PMOD breakout. Common examples:

| Sensor | Measurement | SPI Mode | Max SCK |
|--------|-------------|----------|---------|
| BME280 | Temp / Humidity / Pressure | Mode 0 | 10 MHz |
| MAX31865 | RTD Temperature (PT100/PT1000) | Mode 1 | 5 MHz |
| ADXL345 | 3-axis accelerometer | Mode 3 | 5 MHz |
| MCP3204 | 12-bit ADC (4-channel) | Mode 0 | 1.8 MHz |

**Associated IP:** `pclika_spi_master`

```verilog
pclika_spi_master #(
    .CLK_FREQ (12_000_000),
    .SPI_FREQ (1_000_000),   // 1 MHz — safe for all above
    .CS_AUTO  (1)
) u_spi ( ... );
```

**Pins (PMOD1B):** SCK=pmod1b[0], MOSI=pmod1b[1], MISO=pmod1b[2], CS=pmod1b[3]

---

### PMOD-Servo / PWM Output

Standard RC servo or ESC control at 50 Hz. Connect signal wire to any PMOD pin.

| Servo | Pulse Range | Angle Range |
|-------|-------------|-------------|
| SG90 | 0.5 ms – 2.5 ms | 0° – 180° |
| MG996R | 1.0 ms – 2.0 ms | 0° – 180° |
| Generic ESC | 1.0 ms – 2.0 ms | Off – Full throttle |

**Associated IP:** `pclika_pwm` (CLK_FREQ=12_000_000, PWM_FREQ=50, CNT_WIDTH=18)

| Angle | Duty (cycles @ 12 MHz) |
|-------|------------------------|
| 0° | 6000 (0.5 ms) |
| 90° | 18000 (1.5 ms) |
| 180° | 30000 (2.5 ms) |

**Pins (PMOD1A):** CH0=4, CH1=2, CH2=47, CH3=45

---

### PMOD-Display (SPI OLED / LCD)

| Display | Controller | Resolution | Interface |
|---------|-----------|------------|-----------|
| SSD1306 | Solomon SSD1306 | 128×64 mono OLED | SPI/I2C |
| SSD1309 | Solomon SSD1309 | 128×64 mono OLED | SPI |
| ST7735  | Sitronix ST7735 | 128×160 color LCD | SPI |
| ST7789  | Sitronix ST7789 | 240×240 color LCD | SPI |

**Associated IP:** `pclika_spi_master` — all above use Mode 0 SPI at 8–32 MHz.

For SSD1306/SSD1309: add a 1-cycle DC (data/command) pin toggle between commands and data.

---

## 4. IP Library Reference

Full cross-reference: hardware component → IP module → example.

| Hardware | Interface | IP Module | Status | Example |
|----------|-----------|-----------|--------|---------|
| FT2232H UART | UART 8N1 | `pclika_uart_rx` | ✅ v0.1 | `examples/uart-echo` |
| FT2232H UART | UART 8N1 | `pclika_uart_tx` | ✅ v0.1 | `examples/uart-echo` |
| W25Q128 Flash | SPI Mode 0 | `pclika_spi_master` | ✅ v0.1 | — |
| BME280 | SPI Mode 0 | `pclika_spi_master` | ✅ v0.1 | — |
| MAX31865 | SPI Mode 1 | `pclika_spi_master` | ✅ v0.1 | — |
| SSD1309 OLED | SPI Mode 0 | `pclika_spi_master` | ✅ v0.1 | — |
| ST7789 LCD | SPI Mode 0 | `pclika_spi_master` | ✅ v0.1 | — |
| SG90 / MG996R Servo | PWM 50 Hz | `pclika_pwm` | ✅ v0.1 | `examples/pwm-gen` |
| RGB LED | GPIO/PWM | `pclika_pwm` | ✅ v0.1 | `examples/blink` |
| MPU6050 / BMP280 | I2C | `pclika_i2c_master` | 🔲 Phase 2 | — |
| WS2812B RGB strip | 1-wire 800 kHz | `pclika_ws2812` | 🔲 Phase 2 | — |
| PS/2 Keyboard | PS/2 clock/data | `pclika_ps2_rx` | 🔲 Phase 2 | — |
| VGA output | Parallel RGB | `pclika_vga_sync` | 🔲 Phase 3 | — |
| HDMI output | TMDS | `pclika_hdmi_tx` | 🔲 Phase 3 | — |
| SDRAM | Parallel bus | `pclika_sdram_ctrl` | 🔲 Phase 3 | — |

**Status legend:**
- ✅ v0.1 — implemented in `hdl/rtl/`, synthesizable, tested in simulation
- 🔲 Phase 2 — planned for next IP milestone (I2C + LED strip + input devices)
- 🔲 Phase 3 — long-term roadmap (video output, memory controllers)

---

## 5. Pin Budget Summary (iCEBreaker SG48)

iCE40UP5K SG48 has **39 user I/O** pins. Reserved pins reduce usable count:

| Group | Pins | Count |
|-------|------|-------|
| Clock input | 35 | 1 |
| RGB LED | 39, 40, 41 | 3 |
| UART (FT2232H) | 6, 9 | 2 |
| SPI Flash | 14, 15, 16, 17 | 4 |
| Buttons | 10, 11, 12 | 3 |
| **Reserved total** | | **13** |
| **Available for PMOD / user** | | **26** |

PMOD1A (4 pins) + PMOD1B (4 pins) + PMOD2 (8 pins) = 16 pins — all within budget.

---

## 6. Procurement Links

| Component | Source | Notes |
|-----------|--------|-------|
| iCEBreaker v1.0 | [1bitsquared.com](https://1bitsquared.com/products/icebreaker) | Includes PMOD connectors |
| OSS CAD Suite | [github.com/YosysHQ](https://github.com/YosysHQ/oss-cad-suite-build) | Full toolchain, free |
| BME280 breakout | Adafruit #2652 / generic PMOD | SPI or I2C selectable |
| SSD1309 OLED | Generic SPI OLED (128×64) | Ensure SPI variant, not I2C-only |
| SG90 servo | Generic RC hobby supplier | 3-wire: signal / 5V / GND |
| PMOD UART | Digilent 410-165 | CP2102, 3.3V logic |
