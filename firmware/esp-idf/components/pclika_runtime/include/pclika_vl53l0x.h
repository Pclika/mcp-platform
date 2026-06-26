/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_vl53l0x.h — VL53L0X ToF distance sensor driver
 * PCK-MMXXVI-C4A32096
 *
 * Range: 30–2000 mm (typ.), I2C 400 kHz.
 * Default I2C address: 0x29 (cannot be changed via pin — use XSHUT sequencing).
 *
 * Note: This is a simplified driver that uses the STMicro API sequencing
 * without bundling the full proprietary VL53L0X API.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_VL53L0X_ADDR_DEFAULT  0x29

/** Ranging profile presets */
typedef enum {
    PCLIKA_VL53L0X_PROFILE_DEFAULT      = 0,  /*!< ~33 ms, ~1.2 m range  */
    PCLIKA_VL53L0X_PROFILE_LONG_RANGE   = 1,  /*!< ~33 ms, ~2.0 m range, lower accuracy */
    PCLIKA_VL53L0X_PROFILE_HIGH_SPEED   = 2,  /*!< ~20 ms, ~1.2 m range  */
    PCLIKA_VL53L0X_PROFILE_HIGH_ACCURACY= 3,  /*!< 200 ms, ~1.2 m range, ±3 % accuracy */
} pclika_vl53l0x_profile_t;

typedef struct {
    i2c_port_t               port;
    uint8_t                  addr;
    pclika_vl53l0x_profile_t profile;
    uint32_t                 timing_budget_us;   /*!< Set by profile */
    uint8_t                  stop_variable;      /*!< Calibration byte */
} pclika_vl53l0x_t;

typedef struct {
    uint16_t distance_mm;    /*!< Measured distance in mm; 8190 if out-of-range */
    bool     valid;          /*!< False if range status indicates an error */
    uint8_t  range_status;   /*!< Raw range status byte from device */
    uint16_t signal_rate;    /*!< Signal return rate (Mcps × 256), quality indicator */
} pclika_vl53l0x_data_t;

/**
 * @brief  Initialise VL53L0X: verify model/revision, load default tuning,
 *         run SPAD and reference calibration, set profile.
 * @param[in]  port     I2C port
 * @param[in]  addr     Device address (default 0x29)
 * @param[in]  profile  Ranging profile
 * @param[out] out      Initialised handle
 */
esp_err_t pclika_vl53l0x_init(i2c_port_t port, uint8_t addr,
                               pclika_vl53l0x_profile_t profile,
                               pclika_vl53l0x_t *out);

/**
 * @brief  Perform a single ranging measurement (blocks until complete).
 */
esp_err_t pclika_vl53l0x_read_single(const pclika_vl53l0x_t *dev,
                                      pclika_vl53l0x_data_t *data);

/**
 * @brief  Start continuous ranging mode.
 * @param[in]  period_ms  Inter-measurement period (0 = back-to-back).
 */
esp_err_t pclika_vl53l0x_start_continuous(const pclika_vl53l0x_t *dev,
                                           uint32_t period_ms);

/**
 * @brief  Read latest result from continuous mode (non-blocking).
 *         Returns ESP_ERR_NOT_FINISHED if measurement not ready yet.
 */
esp_err_t pclika_vl53l0x_read_continuous(const pclika_vl53l0x_t *dev,
                                          pclika_vl53l0x_data_t *data);

/**
 * @brief  Stop continuous ranging mode.
 */
esp_err_t pclika_vl53l0x_stop_continuous(const pclika_vl53l0x_t *dev);

/**
 * @brief  Change the I2C address (useful when multiple sensors share bus via XSHUT).
 */
esp_err_t pclika_vl53l0x_set_address(pclika_vl53l0x_t *dev, uint8_t new_addr);

#ifdef __cplusplus
}
#endif
