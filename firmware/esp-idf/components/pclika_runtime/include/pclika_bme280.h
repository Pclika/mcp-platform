/**
 * pclika_bme280.h — BME280 driver public API
 * Seal: PCK-MMXXVI-C4A32096
 */
#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_BME280_ADDR_SDO_LOW  0x76
#define PCLIKA_BME280_ADDR_SDO_HIGH 0x77

typedef struct {
    float temperature_c;  /* °C */
    float humidity_pct;   /* %RH */
    float pressure_pa;    /* Pa (divide by 100 for hPa/mbar) */
} pclika_bme280_data_t;

/**
 * Initialize BME280 over I2C.
 * @param sda_gpio  I2C SDA GPIO number
 * @param scl_gpio  I2C SCL GPIO number
 * @param i2c_addr  Device address (0x76 or 0x77)
 */
esp_err_t pclika_bme280_init(int sda_gpio, int scl_gpio, uint8_t i2c_addr);

/**
 * Read temperature, humidity, and pressure.
 * Must call pclika_bme280_init() first.
 */
esp_err_t pclika_bme280_read(pclika_bme280_data_t *out);

/** Release I2C driver. */
esp_err_t pclika_bme280_deinit(void);

#ifdef __cplusplus
}
#endif
