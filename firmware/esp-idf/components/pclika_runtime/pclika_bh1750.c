/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_bh1750.c — BH1750 driver implementation
 * PCK-MMXXVI-C4A32096
 */
#include "pclika_bh1750.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG         "pclika_bh1750"
#define TIMEOUT_MS  50

/* Opcode bytes */
#define BH1750_POWER_DOWN   0x00
#define BH1750_POWER_ON     0x01
#define BH1750_RESET        0x07

/* Measurement delay per mode in ms */
static uint8_t meas_delay_ms(pclika_bh1750_mode_t mode)
{
    switch (mode) {
        case PCLIKA_BH1750_MODE_CONTINUOUS_LOW:
        case PCLIKA_BH1750_MODE_ONETIME_LOW:
            return 24;
        default:
            return 130;
    }
}

static esp_err_t send_cmd(const pclika_bh1750_t *dev, uint8_t cmd)
{
    return i2c_master_write_to_device(dev->port, dev->addr,
                                      &cmd, 1,
                                      pdMS_TO_TICKS(TIMEOUT_MS));
}

/* -------- Public API -------- */

esp_err_t pclika_bh1750_init(i2c_port_t port, uint8_t addr,
                              pclika_bh1750_mode_t mode,
                              pclika_bh1750_t *out)
{
    out->port = port;
    out->addr = addr;
    out->mode = mode;

    esp_err_t ret = pclika_bh1750_start(out);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "init failed addr=0x%02X: %s", addr, esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "init OK addr=0x%02X mode=0x%02X", addr, mode);
    return ESP_OK;
}

esp_err_t pclika_bh1750_start(const pclika_bh1750_t *dev)
{
    esp_err_t ret = send_cmd(dev, BH1750_POWER_ON);
    if (ret != ESP_OK) return ret;
    /* Allow 1 ms for power-on */
    vTaskDelay(pdMS_TO_TICKS(2));
    return send_cmd(dev, (uint8_t)dev->mode);
}

esp_err_t pclika_bh1750_read(const pclika_bh1750_t *dev, pclika_bh1750_data_t *data)
{
    /* One-shot modes need measurement time before reading */
    bool is_onetime = (dev->mode == PCLIKA_BH1750_MODE_ONETIME_HIGH  ||
                       dev->mode == PCLIKA_BH1750_MODE_ONETIME_HIGH2 ||
                       dev->mode == PCLIKA_BH1750_MODE_ONETIME_LOW);
    if (is_onetime) {
        esp_err_t ret = send_cmd(dev, (uint8_t)dev->mode);
        if (ret != ESP_OK) return ret;
        vTaskDelay(pdMS_TO_TICKS(meas_delay_ms(dev->mode)));
    }

    uint8_t buf[2];
    esp_err_t ret = i2c_master_read_from_device(dev->port, dev->addr,
                                                 buf, sizeof(buf),
                                                 pdMS_TO_TICKS(TIMEOUT_MS));
    if (ret != ESP_OK) return ret;

    data->raw = ((uint16_t)buf[0] << 8) | buf[1];
    /* lux = raw / 1.2 (BH1750 datasheet) */
    data->lux = (float)data->raw / 1.2f;

    /* High-resolution2 mode doubles sensitivity → halve lux */
    if (dev->mode == PCLIKA_BH1750_MODE_CONTINUOUS_HIGH2 ||
        dev->mode == PCLIKA_BH1750_MODE_ONETIME_HIGH2) {
        data->lux /= 2.0f;
    }

    ESP_LOGD(TAG, "raw=%u lux=%.1f", data->raw, data->lux);
    return ESP_OK;
}

esp_err_t pclika_bh1750_power_down(const pclika_bh1750_t *dev)
{
    return send_cmd(dev, BH1750_POWER_DOWN);
}

esp_err_t pclika_bh1750_reset(const pclika_bh1750_t *dev)
{
    /* Reset requires power-on state */
    esp_err_t ret = send_cmd(dev, BH1750_POWER_ON);
    if (ret != ESP_OK) return ret;
    return send_cmd(dev, BH1750_RESET);
}
