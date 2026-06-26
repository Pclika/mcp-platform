/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_sht31.h — SHT31-D high-accuracy temperature/humidity driver
 * PCK-MMXXVI-C4A32096
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/* I2C address — ADDR pin low = 0x44, high = 0x45 */
#define PCLIKA_SHT31_ADDR_LOW   0x44
#define PCLIKA_SHT31_ADDR_HIGH  0x45

/* Repeatability modes */
typedef enum {
    PCLIKA_SHT31_REP_HIGH   = 0,  /*!< High repeatability  — 15 ms */
    PCLIKA_SHT31_REP_MEDIUM = 1,  /*!< Medium repeatability — 6 ms */
    PCLIKA_SHT31_REP_LOW    = 2,  /*!< Low repeatability   —  4 ms */
} pclika_sht31_repeatability_t;

/** Sensor handle */
typedef struct {
    i2c_port_t port;
    uint8_t    addr;
    pclika_sht31_repeatability_t repeatability;
} pclika_sht31_t;

/** Measurement result */
typedef struct {
    float temperature_c;    /*!< Temperature in °C  */
    float humidity_pct;     /*!< Relative humidity 0–100 % */
} pclika_sht31_data_t;

/**
 * @brief Initialise SHT31 and verify communication.
 * @param[in]  port         I2C port number
 * @param[in]  addr         Device address (PCLIKA_SHT31_ADDR_LOW or HIGH)
 * @param[in]  repeatability  Measurement repeatability
 * @param[out] out          Initialised handle
 * @return ESP_OK on success
 */
esp_err_t pclika_sht31_init(i2c_port_t port, uint8_t addr,
                             pclika_sht31_repeatability_t repeatability,
                             pclika_sht31_t *out);

/**
 * @brief Read temperature and humidity.
 * @param[in]  dev   Handle returned by pclika_sht31_init()
 * @param[out] data  Measurement result
 * @return ESP_OK, ESP_ERR_INVALID_CRC on checksum fail
 */
esp_err_t pclika_sht31_read(const pclika_sht31_t *dev, pclika_sht31_data_t *data);

/**
 * @brief Soft-reset the SHT31.
 */
esp_err_t pclika_sht31_reset(const pclika_sht31_t *dev);

/**
 * @brief Read and check status register.
 * @param[out] status  Raw 16-bit status register value
 */
esp_err_t pclika_sht31_status(const pclika_sht31_t *dev, uint16_t *status);

/**
 * @brief Clear status register.
 */
esp_err_t pclika_sht31_clear_status(const pclika_sht31_t *dev);

#ifdef __cplusplus
}
#endif
