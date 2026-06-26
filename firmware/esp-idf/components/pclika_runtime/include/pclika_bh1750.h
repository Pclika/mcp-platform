/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_bh1750.h — BH1750 ambient light sensor driver
 * PCK-MMXXVI-C4A32096
 *
 * I2C, up to 1 Mbit/s. ADDR pin low = 0x23, high = 0x5C.
 * Output: 16-bit raw count → lux = raw / 1.2
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_BH1750_ADDR_LOW  0x23   /*!< ADDR pin → GND */
#define PCLIKA_BH1750_ADDR_HIGH 0x5C   /*!< ADDR pin → VCC */

typedef enum {
    PCLIKA_BH1750_MODE_CONTINUOUS_HIGH  = 0x10,  /*!< 1 lux   resolution, 120 ms  */
    PCLIKA_BH1750_MODE_CONTINUOUS_HIGH2 = 0x11,  /*!< 0.5 lux resolution, 120 ms  */
    PCLIKA_BH1750_MODE_CONTINUOUS_LOW   = 0x13,  /*!< 4 lux   resolution,  16 ms  */
    PCLIKA_BH1750_MODE_ONETIME_HIGH     = 0x20,  /*!< One-shot, 1 lux   */
    PCLIKA_BH1750_MODE_ONETIME_HIGH2    = 0x21,  /*!< One-shot, 0.5 lux */
    PCLIKA_BH1750_MODE_ONETIME_LOW      = 0x23,  /*!< One-shot, 4 lux   */
} pclika_bh1750_mode_t;

typedef struct {
    i2c_port_t           port;
    uint8_t              addr;
    pclika_bh1750_mode_t mode;
} pclika_bh1750_t;

typedef struct {
    float    lux;       /*!< Illuminance in lux */
    uint16_t raw;       /*!< Raw 16-bit register value */
} pclika_bh1750_data_t;

/**
 * @brief  Initialise BH1750 and set measurement mode.
 */
esp_err_t pclika_bh1750_init(i2c_port_t port, uint8_t addr,
                              pclika_bh1750_mode_t mode,
                              pclika_bh1750_t *out);

/**
 * @brief  Power-on and start continuous measurement.
 */
esp_err_t pclika_bh1750_start(const pclika_bh1750_t *dev);

/**
 * @brief  Read latest lux value (blocks for measurement time in one-shot mode).
 */
esp_err_t pclika_bh1750_read(const pclika_bh1750_t *dev, pclika_bh1750_data_t *data);

/**
 * @brief  Power-down the sensor (retain registers).
 */
esp_err_t pclika_bh1750_power_down(const pclika_bh1750_t *dev);

/**
 * @brief  Reset data register (requires power-on state).
 */
esp_err_t pclika_bh1750_reset(const pclika_bh1750_t *dev);

#ifdef __cplusplus
}
#endif
