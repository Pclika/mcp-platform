# Sensor Debugging Prompt

Paste this into your AI tool when a sensor is not responding or returning incorrect values.

---

```
I am debugging a sensor on the Pclika MCP Platform (ESP32-S3, ESP-IDF v5.x).

## Symptom

[Describe what is happening: no response, garbage values, CRC errors, wrong units, etc.]

## Sensor

- **Type:** [e.g. SHT31 / BH1750 / MPU6050 / VL53L0X / BME280 / DHT22]
- **Interface:** [I2C / SPI / one-wire]
- **Address / CS pin:** [e.g. 0x44 / GPIO5]
- **Pull-ups on SDA/SCL:** [yes / no / unknown]

## Firmware call

```c
// Paste the relevant sensor_read or driver call here
```

## Serial output

```
// Paste relevant idf.py monitor output here
```

## What I have already tried

[List steps already taken]

---

Please help me:
1. Identify the most likely cause of the failure given the sensor type and symptom.
2. List the specific register or timing sequence to verify first (with datasheet reference if applicable).
3. Provide an I2C scanner snippet I can add to app_main() to confirm the device is visible on the bus.
4. If CRC errors are suspected, provide the CRC-8 polynomial and verification code for this specific sensor.
5. Suggest any wiring or power supply checks specific to this component.

Use the Pclika driver at `firmware/esp-idf/components/pclika_runtime/src/pclika_<sensor>.c` as the reference implementation.
```
