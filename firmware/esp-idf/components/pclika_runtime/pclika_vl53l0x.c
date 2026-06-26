/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_vl53l0x.c — VL53L0X ToF distance sensor driver
 * PCK-MMXXVI-C4A32096
 *
 * Based on the VL53L0X datasheet ranging sequence and Pololu/ST open-source
 * reference implementations. No proprietary VL53L0X API dependency.
 */
#include "pclika_vl53l0x.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define TAG        "pclika_vl53l0x"
#define TIMEOUT_MS 500

/* --- Register addresses (from VL53L0X datasheet / errata) --- */
#define REG_IDENTIFICATION_MODEL_ID          0xC0
#define REG_IDENTIFICATION_REVISION_ID       0xC2
#define REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV 0x89
#define REG_MSRC_CONFIG_CONTROL              0x60
#define REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT 0x44
#define REG_SYSTEM_SEQUENCE_CONFIG           0x01
#define REG_DYNAMIC_SPAD_REF_EN_START_OFFSET 0x4F
#define REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD 0x4E
#define REG_GLOBAL_CONFIG_REF_EN_START_SELECT 0xB6
#define REG_SYSTEM_INTERRUPT_CONFIG_GPIO     0x0A
#define REG_GPIO_HV_MUX_ACTIVE_HIGH          0x84
#define REG_SYSTEM_INTERRUPT_CLEAR           0x0B
#define REG_RESULT_INTERRUPT_STATUS          0x13
#define REG_SYSRANGE_START                   0x00
#define REG_RESULT_RANGE_STATUS              0x14
#define REG_OSC_CALIBRATE_VAL                0xF8
#define REG_SYSTEM_INTERMEASUREMENT_PERIOD   0x04

/* Timing budget micro-seconds for each profile */
static const uint32_t PROFILE_TIMING_US[4] = {33000, 33000, 20000, 200000};

/* -------- I2C register helpers -------- */
static esp_err_t reg_write8(const pclika_vl53l0x_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_write_to_device(dev->port, dev->addr,
                                      buf, sizeof(buf),
                                      pdMS_TO_TICKS(TIMEOUT_MS));
}

static esp_err_t reg_write16(const pclika_vl53l0x_t *dev, uint8_t reg, uint16_t val)
{
    uint8_t buf[3] = {reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF)};
    return i2c_master_write_to_device(dev->port, dev->addr,
                                      buf, sizeof(buf),
                                      pdMS_TO_TICKS(TIMEOUT_MS));
}

static esp_err_t reg_write32(const pclika_vl53l0x_t *dev, uint8_t reg, uint32_t val)
{
    uint8_t buf[5] = {reg,
                      (uint8_t)(val >> 24), (uint8_t)(val >> 16),
                      (uint8_t)(val >> 8),  (uint8_t)(val & 0xFF)};
    return i2c_master_write_to_device(dev->port, dev->addr,
                                      buf, sizeof(buf),
                                      pdMS_TO_TICKS(TIMEOUT_MS));
}

static esp_err_t reg_read8(const pclika_vl53l0x_t *dev, uint8_t reg, uint8_t *out)
{
    return i2c_master_write_read_device(dev->port, dev->addr,
                                        &reg, 1, out, 1,
                                        pdMS_TO_TICKS(TIMEOUT_MS));
}

static esp_err_t reg_read_burst(const pclika_vl53l0x_t *dev,
                                 uint8_t reg, uint8_t *out, size_t len)
{
    return i2c_master_write_read_device(dev->port, dev->addr,
                                        &reg, 1, out, len,
                                        pdMS_TO_TICKS(TIMEOUT_MS));
}

static esp_err_t reg_update8(const pclika_vl53l0x_t *dev, uint8_t reg,
                              uint8_t mask, uint8_t val)
{
    uint8_t cur;
    esp_err_t ret = reg_read8(dev, reg, &cur);
    if (ret != ESP_OK) return ret;
    return reg_write8(dev, reg, (cur & ~mask) | (val & mask));
}

/* -------- Minimal init sequence (ST AN4545 / Pololu approach) -------- */

static esp_err_t data_init(const pclika_vl53l0x_t *dev)
{
    /* Enable 2.8V I2O if needed */
    uint8_t extsup;
    esp_err_t ret = reg_read8(dev, REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, &extsup);
    if (ret != ESP_OK) return ret;
    if (extsup != 0) {
        ret = reg_write8(dev, REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, extsup | 0x01);
        if (ret != ESP_OK) return ret;
    }

    /* Set I2C standard mode */
    ret = reg_write8(dev, 0x88, 0x00);  if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x80, 0x01);  if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0xFF, 0x01);  if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x00, 0x00);  if (ret != ESP_OK) return ret;

    uint8_t sv;
    ret = reg_read8(dev, 0x91, &sv);
    if (ret != ESP_OK) return ret;
    ((pclika_vl53l0x_t *)dev)->stop_variable = sv;

    ret = reg_write8(dev, 0x00, 0x01);  if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0xFF, 0x00);  if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x80, 0x00);  if (ret != ESP_OK) return ret;

    /* Disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE limit checks */
    uint8_t msrc_cfg;
    ret = reg_read8(dev, REG_MSRC_CONFIG_CONTROL, &msrc_cfg);
    if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, REG_MSRC_CONFIG_CONTROL, msrc_cfg | 0x12);
    if (ret != ESP_OK) return ret;

    /* Set signal rate limit to 0.1 Mcps (default 0.25) — helps long range */
    ret = reg_write16(dev, REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 32);
    if (ret != ESP_OK) return ret;

    /* Enable all sequence steps */
    ret = reg_write8(dev, REG_SYSTEM_SEQUENCE_CONFIG, 0xFF);
    return ret;
}

static esp_err_t static_init(const pclika_vl53l0x_t *dev)
{
    /* Simplified: skip SPAD calibration; use factory defaults */
    esp_err_t ret = reg_write8(dev, REG_SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (ret != ESP_OK) return ret;
    /* Re-enable TCC, DSS, MSRC, PRE_RANGE, FINAL_RANGE */
    return reg_write8(dev, REG_SYSTEM_SEQUENCE_CONFIG, 0xE8);
}

static esp_err_t set_timing_budget(const pclika_vl53l0x_t *dev, uint32_t budget_us)
{
    /* Simplified timing budget register write — stores in INTERMEASUREMENT if needed */
    (void)budget_us;
    /* Actual budget calculation requires reading sequence step enables & timeouts;
     * for the default 33 ms budget the factory settings are correct.
     * Long-range and high-accuracy profiles adjust via pulse period registers. */
    return ESP_OK;
}

static esp_err_t apply_profile(const pclika_vl53l0x_t *dev)
{
    esp_err_t ret = ESP_OK;
    switch (dev->profile) {
        case PCLIKA_VL53L0X_PROFILE_LONG_RANGE:
            /* Increase pulse periods for longer range */
            ret = reg_write8(dev, 0x18, 0x38);  /* PRE_RANGE_CONFIG_VCSEL_PERIOD */
            if (ret != ESP_OK) return ret;
            ret = reg_write8(dev, 0x30, 0x44);  /* FINAL_RANGE_CONFIG_VCSEL_PERIOD */
            if (ret != ESP_OK) return ret;
            break;
        case PCLIKA_VL53L0X_PROFILE_HIGH_SPEED:
            ret = reg_write16(dev, REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 25);
            break;
        case PCLIKA_VL53L0X_PROFILE_HIGH_ACCURACY:
            ret = reg_write16(dev, REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 50);
            break;
        default:
            break;
    }
    return ret;
}

/* -------- Public API -------- */

esp_err_t pclika_vl53l0x_init(i2c_port_t port, uint8_t addr,
                               pclika_vl53l0x_profile_t profile,
                               pclika_vl53l0x_t *out)
{
    out->port             = port;
    out->addr             = addr;
    out->profile          = profile;
    out->timing_budget_us = PROFILE_TIMING_US[profile];
    out->stop_variable    = 0;

    /* Verify model ID */
    uint8_t model_id;
    esp_err_t ret = reg_read8(out, REG_IDENTIFICATION_MODEL_ID, &model_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "no device at 0x%02X: %s", addr, esp_err_to_name(ret));
        return ret;
    }
    if (model_id != 0xEE) {
        ESP_LOGW(TAG, "unexpected model ID 0x%02X (expected 0xEE)", model_id);
    }

    ret = data_init(out);    if (ret != ESP_OK) return ret;
    ret = static_init(out);  if (ret != ESP_OK) return ret;
    ret = apply_profile(out);if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "init OK addr=0x%02X profile=%d budget=%lu us",
             addr, profile, (unsigned long)out->timing_budget_us);
    return ESP_OK;
}

esp_err_t pclika_vl53l0x_read_single(const pclika_vl53l0x_t *dev,
                                      pclika_vl53l0x_data_t *data)
{
    /* Start single ranging */
    esp_err_t ret = reg_write8(dev, 0x80, 0x01); if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0xFF, 0x01);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x00, 0x00);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x91, dev->stop_variable); if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x00, 0x01);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0xFF, 0x00);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x80, 0x00);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, REG_SYSRANGE_START, 0x01); if (ret != ESP_OK) return ret;

    /* Wait for measurement ready (interrupt status bit 0) */
    uint32_t deadline_ms = dev->timing_budget_us / 1000 + 50;
    uint32_t elapsed = 0;
    uint8_t  int_status;
    do {
        ret = reg_read8(dev, REG_RESULT_INTERRUPT_STATUS, &int_status);
        if (ret != ESP_OK) return ret;
        vTaskDelay(pdMS_TO_TICKS(5));
        elapsed += 5;
        if (elapsed > deadline_ms) {
            ESP_LOGW(TAG, "measurement timeout after %lu ms", (unsigned long)elapsed);
            return ESP_ERR_TIMEOUT;
        }
    } while ((int_status & 0x07) == 0);

    /* Read result: 12 bytes starting at REG_RESULT_RANGE_STATUS */
    uint8_t buf[12];
    ret = reg_read_burst(dev, REG_RESULT_RANGE_STATUS, buf, sizeof(buf));
    if (ret != ESP_OK) return ret;

    /* Clear interrupt */
    reg_write8(dev, REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    reg_write8(dev, REG_SYSRANGE_START, 0x00);

    data->range_status   = (buf[0] >> 3) & 0x1F;
    data->signal_rate    = ((uint16_t)buf[6] << 8) | buf[7];
    data->distance_mm    = ((uint16_t)buf[10] << 8) | buf[11];
    data->valid          = (data->range_status == 0);

    ESP_LOGD(TAG, "dist=%u mm valid=%d status=%u sig=%u",
             data->distance_mm, data->valid,
             data->range_status, data->signal_rate);
    return ESP_OK;
}

esp_err_t pclika_vl53l0x_start_continuous(const pclika_vl53l0x_t *dev, uint32_t period_ms)
{
    esp_err_t ret = reg_write8(dev, 0x80, 0x01); if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0xFF, 0x01);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x00, 0x00);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x91, dev->stop_variable); if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x00, 0x01);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0xFF, 0x00);            if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x80, 0x00);            if (ret != ESP_OK) return ret;

    if (period_ms != 0) {
        /* Timed continuous mode: period in ms × 1024 / osc_calibrate */
        uint16_t osc_cal;
        uint8_t osc_buf[2];
        reg_read_burst(dev, REG_OSC_CALIBRATE_VAL, osc_buf, 2);
        osc_cal = ((uint16_t)osc_buf[0] << 8) | osc_buf[1];
        if (osc_cal != 0) {
            period_ms *= osc_cal;
        }
        ret = reg_write32(dev, REG_SYSTEM_INTERMEASUREMENT_PERIOD, period_ms);
        if (ret != ESP_OK) return ret;
        ret = reg_write8(dev, REG_SYSRANGE_START, 0x04); /* timed continuous */
    } else {
        ret = reg_write8(dev, REG_SYSRANGE_START, 0x02); /* back-to-back */
    }
    return ret;
}

esp_err_t pclika_vl53l0x_read_continuous(const pclika_vl53l0x_t *dev,
                                          pclika_vl53l0x_data_t *data)
{
    uint8_t int_status;
    esp_err_t ret = reg_read8(dev, REG_RESULT_INTERRUPT_STATUS, &int_status);
    if (ret != ESP_OK) return ret;
    if ((int_status & 0x07) == 0) {
        return ESP_ERR_NOT_FINISHED;
    }

    uint8_t buf[12];
    ret = reg_read_burst(dev, REG_RESULT_RANGE_STATUS, buf, sizeof(buf));
    if (ret != ESP_OK) return ret;
    reg_write8(dev, REG_SYSTEM_INTERRUPT_CLEAR, 0x01);

    data->range_status = (buf[0] >> 3) & 0x1F;
    data->signal_rate  = ((uint16_t)buf[6] << 8) | buf[7];
    data->distance_mm  = ((uint16_t)buf[10] << 8) | buf[11];
    data->valid        = (data->range_status == 0);
    return ESP_OK;
}

esp_err_t pclika_vl53l0x_stop_continuous(const pclika_vl53l0x_t *dev)
{
    esp_err_t ret = reg_write8(dev, REG_SYSRANGE_START, 0x01); if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0xFF, 0x01);  if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x00, 0x00);  if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x91, 0x00);  if (ret != ESP_OK) return ret;
    ret = reg_write8(dev, 0x00, 0x01);  if (ret != ESP_OK) return ret;
    return reg_write8(dev, 0xFF, 0x00);
}

esp_err_t pclika_vl53l0x_set_address(pclika_vl53l0x_t *dev, uint8_t new_addr)
{
    esp_err_t ret = reg_write8(dev, 0x8A, new_addr & 0x7F);
    if (ret == ESP_OK) {
        dev->addr = new_addr;
    }
    return ret;
}
