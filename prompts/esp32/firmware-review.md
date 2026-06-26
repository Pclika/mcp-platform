# Firmware Code Review Prompt

Paste this when asking an AI to review an ESP-IDF component or example for the Pclika platform.

---

```
Please review the following ESP-IDF firmware code for the Pclika MCP Platform.

## Code under review

```c
// Paste the code here, or reference the file path:
// firmware/esp-idf/components/pclika_runtime/src/<filename>.c
```

## Context

- **Platform:** ESP32-S3, ESP-IDF v5.x, FreeRTOS
- **Component type:** [sensor driver / display driver / MCP tool handler / example main]
- **Goal:** [What this code is supposed to do]

---

Review for the following:

### Correctness
- Are I2C/SPI register addresses and sequences correct per the datasheet?
- Is CRC computed correctly (SHT31: poly 0x31, init 0xFF; Modbus RTU: poly 0xA001, init 0xFFFF)?
- Are units correct? (SHT31: temp = -45 + 175 * raw/65535; BH1750: lux = raw/1.2, HIGH2 mode /= 2; MPU6050: LSB divisors per range)
- Are error return codes from ESP-IDF APIs checked?

### Memory safety
- Are stack allocations appropriately sized?
- Are heap allocations freed on all code paths (including error paths)?
- Is the cJSON result pointer set to NULL before early returns?
- Are string buffers null-terminated and sized correctly?

### FreeRTOS safety
- Are delays using `pdMS_TO_TICKS()` not raw tick counts?
- Are shared data structures protected with mutexes or atomic ops if accessed from multiple tasks?
- Are ISR-safe queue/semaphore variants used from interrupt context?

### Pclika conventions
- Does the tool handler signature match `pclika_err_t fn(cJSON *params, cJSON **result, void *ctx)`?
- Are error codes returning the correct `PCLIKA_ERR_*` value?
- Does the component declare all its dependencies in `CMakeLists.txt` REQUIRES?
- Are log tags using the component name string?

### MCP compatibility
- Does the output JSON match the schema in `bridge/tool-schemas/<family>-tools.json`?
- Are required output fields always present, even on partial success?
- Is the `timestamp_ms` field populated from `esp_timer_get_time() / 1000`?

Provide specific line-level feedback and corrected code snippets where applicable.
```
