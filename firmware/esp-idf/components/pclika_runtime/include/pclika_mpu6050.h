/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_mpu6050.h — MPU6050 6-axis IMU driver
 * PCK-MMXXVI-C4A32096
 *
 * Provides: accelerometer (m/s²), gyroscope (°/s), die temperature (°C).
 * I2C addr: 0x68 (AD0 low) or 0x69 (AD0 high).
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_MPU6050_ADDR_LOW   0x68   /*!< AD0 pin → GND */
#define PCLIKA_MPU6050_ADDR_HIGH  0x69   /*!< AD0 pin → VCC */

/** Accelerometer full-scale range */
typedef enum {
    PCLIKA_MPU6050_ACCEL_2G  = 0,   /*!< ±2 g  */
    PCLIKA_MPU6050_ACCEL_4G  = 1,   /*!< ±4 g  */
    PCLIKA_MPU6050_ACCEL_8G  = 2,   /*!< ±8 g  */
    PCLIKA_MPU6050_ACCEL_16G = 3,   /*!< ±16 g */
} pclika_mpu6050_accel_range_t;

/** Gyroscope full-scale range */
typedef enum {
    PCLIKA_MPU6050_GYRO_250DPS  = 0, /*!< ±250  °/s */
    PCLIKA_MPU6050_GYRO_500DPS  = 1, /*!< ±500  °/s */
    PCLIKA_MPU6050_GYRO_1000DPS = 2, /*!< ±1000 °/s */
    PCLIKA_MPU6050_GYRO_2000DPS = 3, /*!< ±2000 °/s */
} pclika_mpu6050_gyro_range_t;

typedef struct {
    i2c_port_t                   port;
    uint8_t                      addr;
    pclika_mpu6050_accel_range_t accel_range;
    pclika_mpu6050_gyro_range_t  gyro_range;
} pclika_mpu6050_t;

/** 3-axis vector */
typedef struct { float x, y, z; } pclika_vec3_t;

typedef struct {
    pclika_vec3_t accel;    /*!< Acceleration in m/s²            */
    pclika_vec3_t gyro;     /*!< Angular velocity in °/s         */
    float         temp_c;   /*!< Die temperature in °C           */
} pclika_mpu6050_data_t;

/**
 * @brief  Wake, configure ranges, verify WHO_AM_I.
 */
esp_err_t pclika_mpu6050_init(i2c_port_t port, uint8_t addr,
                               pclika_mpu6050_accel_range_t accel_range,
                               pclika_mpu6050_gyro_range_t  gyro_range,
                               pclika_mpu6050_t *out);

/**
 * @brief  Read all six axes + temperature in one burst.
 */
esp_err_t pclika_mpu6050_read(const pclika_mpu6050_t *dev, pclika_mpu6050_data_t *data);

/**
 * @brief  Put MPU6050 into sleep mode.
 */
esp_err_t pclika_mpu6050_sleep(const pclika_mpu6050_t *dev);

/**
 * @brief  Wake MPU6050 from sleep.
 */
esp_err_t pclika_mpu6050_wake(const pclika_mpu6050_t *dev);

/**
 * @brief  Set DLPF bandwidth (0=260 Hz … 6=5 Hz).
 */
esp_err_t pclika_mpu6050_set_dlpf(const pclika_mpu6050_t *dev, uint8_t bandwidth);

#ifdef __cplusplus
}
#endif
