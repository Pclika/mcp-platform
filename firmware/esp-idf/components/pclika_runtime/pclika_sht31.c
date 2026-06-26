/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_sht31.c — SHT31-D driver implementation
 * PCK-MMXXVI-C4A32096
 *
 * Protocol: I2C, 2-byte command + fetch after measurement delay.
 * CRC-8: poly 0x31, init 0xFF, reflected=no, xorout=0x00.
 */
#include "pclika_sht31.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define TAG "pclika_sht31"
#define TIMEOUT_MS  100

/* Single-shot commands (clock stretching disabled) */
static const uint8_t CMD_MEAS_HIGH[2]   = {0x24, 0x00};
static const uint8_t CMD_MEAS_MEDIUM[2] = {0x24, 0x0B};
static const uint8_t CMD_MEAS_LOW[2]    = {0x24, 0x16};
static const uint8_t CMD_SOFT_RESET[2]  = {0x30, 0xA2};
static const uint8_t CMD_READ_STATUS[2] = {0xF3, 0x2D};
static const uint8_t CMD_CLEAR_STATUS[2]= {0x30, 0x41};

/* Measurement delays in ms */
static const uint8_t MEAS_DELAY_MS[3] = {15, 6, 4};

/* ---------- CRC-8 (poly 0x31, init 0xFF) ---------- */
static uint8_t sht31_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
        }
    }
    return crc;
}

/* ---------- Low-level I2C helpers ---------- */
static esp_err_t i2c_write(const pclika_sht31_t *dev, const uint8_t *cmd, size_t len)
{
    return i2c_master_write_to_device(dev->port, dev->addr,
                                      cmd, len,
                                      pdMS_TO_TICKS(TIMEOUT_MS));
}

static esp_err_t i2c_read(const pclika_sht31_t *dev, uint8_t *buf, size_t len)
{
    return i2c_master_read_from_device(dev->port, dev->addr,
                                       buf, len,
                                       pdMS_TO_TICKS(TIMEOUT_MS));
}

/* ---------- Public API ---------- */

esp_err_t pclika_sht31_init(i2c_port_t port, uint8_t addr,
                             pclika_sht31_repeatability_t repeatability,
                             pclika_sht31_t *out)
{
    out->port         = port;
    out->addr         = addr;
    out->repeatability = repeatability;

    /* Soft reset to clear any stale state */
    esp_err_t ret = pclika_sht31_reset(out);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "reset failed: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(2));  /* 1 ms reset duration */

    /* Verify by reading status */
    uint16_t status = 0;
    ret = pclika_sht31_status(out, &status);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "no response at 0x%02X: %s", addr, esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "init OK addr=0x%02X status=0x%04X", addr, status);
    return ESP_OK;
}

esp_err_t pclika_sht31_read(const pclika_sht31_t *dev, pclika_sht31_data_t *data)
{
    /* Send measurement command */
    const uint8_t *cmd;
    switch (dev->repeatability) {
        case PCLIKA_SHT31_REP_MEDIUM: cmd = CMD_MEAS_MEDIUM; break;
        case PCLIKA_SHT31_REP_LOW:    cmd = CMD_MEAS_LOW;    break;
        default:                       cmd = CMD_MEAS_HIGH;   break;
    }
    esp_err_t ret = i2c_write(dev, cmd, 2);
    if (ret != ESP_OK) return ret;

    /* Wait for conversion */
    vTaskDelay(pdMS_TO_TICKS(MEAS_DELAY_MS[dev->repeatability] + 1));

    /* Fetch 6 bytes: T_MSB T_LSB T_CRC H_MSB H_LSB H_CRC */
    uint8_t buf[6];
    ret = i2c_read(dev, buf, sizeof(buf));
    if (ret != ESP_OK) return ret;

    /* Verify CRCs */
    if (sht31_crc8(buf, 2) != buf[2]) {
        ESP_LOGW(TAG, "temperature CRC mismatch");
        return ESP_ERR_INVALID_CRC;
    }
    if (sht31_crc8(buf + 3, 2) != buf[5]) {
        ESP_LOGW(TAG, "humidity CRC mismatch");
        return ESP_ERR_INVALID_CRC;
    }

    /* Convert — SHT31 datasheet equations */
    uint16_t raw_t = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t raw_h = ((uint16_t)buf[3] << 8) | buf[4];

    data->temperature_c = -45.0f + 175.0f * ((float)raw_t / 65535.0f);
    data->humidity_pct  = 100.0f  * ((float)raw_h / 65535.0f);

    /* Clamp humidity to valid range */
    if (data->humidity_pct < 0.0f)   data->humidity_pct = 0.0f;
    if (data->humidity_pct > 100.0f) data->humidity_pct = 100.0f;

    ESP_LOGD(TAG, "T=%.2f°C H=%.2f%%", data->temperature_c, data->humidity_pct);
    return ESP_OK;
}

esp_err_t pclika_sht31_reset(const pclika_sht31_t *dev)
{
    return i2c_write(dev, CMD_SOFT_RESET, 2);
}

esp_err_t pclika_sht31_status(const pclika_sht31_t *dev, uint16_t *status)
{
    esp_err_t ret = i2c_write(dev, CMD_READ_STATUS, 2);
    if (ret != ESP_OK) return ret;

    uint8_t buf[3];
    ret = i2c_read(dev, buf, sizeof(buf));
    if (ret != ESP_OK) return ret;

    if (sht31_crc8(buf, 2) != buf[2]) {
        return ESP_ERR_INVALID_CRC;
    }
    *status = ((uint16_t)buf[0] << 8) | buf[1];
    return ESP_OK;
}

esp_err_t pclika_sht31_clear_status(const pclika_sht31_t *dev)
{
    return i2c_write(dev, CMD_CLEAR_STATUS, 2);
}
