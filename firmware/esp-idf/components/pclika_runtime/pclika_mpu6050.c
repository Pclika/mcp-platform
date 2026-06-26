/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_mpu6050.c — MPU6050 6-axis IMU driver
 * PCK-MMXXVI-C4A32096
 */
#include "pclika_mpu6050.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG        "pclika_mpu6050"
#define TIMEOUT_MS 100

/* Register map */
#define REG_SMPLRT_DIV   0x19
#define REG_CONFIG       0x1A
#define REG_GYRO_CONFIG  0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_ACCEL_XOUT_H 0x3B   /* Burst: AX AX AY AY AZ AZ TEMP TEMP GX GX GY GY GZ GZ */
#define REG_PWR_MGMT_1   0x6B
#define REG_WHO_AM_I     0x75

#define WHO_AM_I_VALUE   0x68   /* Fixed ID per datasheet */

/* Sensitivity divisors (raw counts per unit) */
static const float ACCEL_LSB[4] = {16384.0f, 8192.0f, 4096.0f, 2048.0f};  /* counts/g */
static const float GYRO_LSB[4]  = {131.0f,   65.5f,   32.8f,   16.4f};    /* counts/(°/s) */
#define GRAVITY_MS2      9.80665f

/* -------- I2C helpers -------- */
static esp_err_t reg_write(const pclika_mpu6050_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_write_to_device(dev->port, dev->addr,
                                      buf, sizeof(buf),
                                      pdMS_TO_TICKS(TIMEOUT_MS));
}

static esp_err_t reg_read_burst(const pclika_mpu6050_t *dev,
                                 uint8_t reg, uint8_t *out, size_t len)
{
    return i2c_master_write_read_device(dev->port, dev->addr,
                                        &reg, 1,
                                        out, len,
                                        pdMS_TO_TICKS(TIMEOUT_MS));
}

static inline int16_t combine(uint8_t hi, uint8_t lo)
{
    return (int16_t)((uint16_t)hi << 8 | lo);
}

/* -------- Public API -------- */

esp_err_t pclika_mpu6050_init(i2c_port_t port, uint8_t addr,
                               pclika_mpu6050_accel_range_t accel_range,
                               pclika_mpu6050_gyro_range_t  gyro_range,
                               pclika_mpu6050_t *out)
{
    out->port        = port;
    out->addr        = addr;
    out->accel_range = accel_range;
    out->gyro_range  = gyro_range;

    /* Wake device (clear SLEEP bit), use internal oscillator */
    esp_err_t ret = reg_write(out, REG_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "wake failed: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(100));  /* startup delay */

    /* Verify WHO_AM_I */
    uint8_t who = 0;
    ret = reg_read_burst(out, REG_WHO_AM_I, &who, 1);
    if (ret != ESP_OK || who != WHO_AM_I_VALUE) {
        ESP_LOGE(TAG, "WHO_AM_I=0x%02X expected 0x%02X", who, WHO_AM_I_VALUE);
        return (ret == ESP_OK) ? ESP_ERR_NOT_FOUND : ret;
    }

    /* Sample rate divider = 0 → 1 kHz */
    ret = reg_write(out, REG_SMPLRT_DIV, 0x00);
    if (ret != ESP_OK) return ret;

    /* DLPF: 94 Hz bandwidth */
    ret = reg_write(out, REG_CONFIG, 0x02);
    if (ret != ESP_OK) return ret;

    /* Gyro full-scale */
    ret = reg_write(out, REG_GYRO_CONFIG, (uint8_t)(gyro_range << 3));
    if (ret != ESP_OK) return ret;

    /* Accel full-scale */
    ret = reg_write(out, REG_ACCEL_CONFIG, (uint8_t)(accel_range << 3));
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "init OK addr=0x%02X accel=±%dg gyro=±%d°/s",
             addr,
             (int[]){2, 4, 8, 16}[accel_range],
             (int[]){250, 500, 1000, 2000}[gyro_range]);
    return ESP_OK;
}

esp_err_t pclika_mpu6050_read(const pclika_mpu6050_t *dev, pclika_mpu6050_data_t *data)
{
    /* Burst-read 14 bytes: ACCEL(6) + TEMP(2) + GYRO(6) */
    uint8_t buf[14];
    esp_err_t ret = reg_read_burst(dev, REG_ACCEL_XOUT_H, buf, sizeof(buf));
    if (ret != ESP_OK) return ret;

    float a_lsb = ACCEL_LSB[dev->accel_range];
    float g_lsb = GYRO_LSB[dev->gyro_range];

    data->accel.x = (float)combine(buf[0],  buf[1])  / a_lsb * GRAVITY_MS2;
    data->accel.y = (float)combine(buf[2],  buf[3])  / a_lsb * GRAVITY_MS2;
    data->accel.z = (float)combine(buf[4],  buf[5])  / a_lsb * GRAVITY_MS2;

    int16_t raw_t = combine(buf[6], buf[7]);
    data->temp_c  = (float)raw_t / 340.0f + 36.53f;  /* MPU6050 datasheet */

    data->gyro.x  = (float)combine(buf[8],  buf[9])  / g_lsb;
    data->gyro.y  = (float)combine(buf[10], buf[11]) / g_lsb;
    data->gyro.z  = (float)combine(buf[12], buf[13]) / g_lsb;

    ESP_LOGD(TAG, "AX=%.2f AY=%.2f AZ=%.2f GX=%.2f GY=%.2f GZ=%.2f T=%.1f",
             data->accel.x, data->accel.y, data->accel.z,
             data->gyro.x,  data->gyro.y,  data->gyro.z,
             data->temp_c);
    return ESP_OK;
}

esp_err_t pclika_mpu6050_sleep(const pclika_mpu6050_t *dev)
{
    return reg_write(dev, REG_PWR_MGMT_1, 0x40);  /* SLEEP=1 */
}

esp_err_t pclika_mpu6050_wake(const pclika_mpu6050_t *dev)
{
    return reg_write(dev, REG_PWR_MGMT_1, 0x00);
}

esp_err_t pclika_mpu6050_set_dlpf(const pclika_mpu6050_t *dev, uint8_t bandwidth)
{
    if (bandwidth > 6) return ESP_ERR_INVALID_ARG;
    return reg_write(dev, REG_CONFIG, bandwidth & 0x07);
}
