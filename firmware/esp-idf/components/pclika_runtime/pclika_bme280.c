/**
 * pclika_bme280.c — BME280 Temperature/Humidity/Pressure Driver
 *
 * Uses I2C bus (default addr 0x76, SDO=GND; 0x77, SDO=3V3).
 * Compensates raw ADC values using Bosch factory calibration data.
 *
 * API:
 *   pclika_bme280_init(sda, scl, addr)
 *   pclika_bme280_read(out)   → pclika_bme280_data_t
 *   pclika_bme280_deinit()
 *
 * Seal: PCK-MMXXVI-C4A32096
 */

#include "pclika_bme280.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>
#include <math.h>

static const char *TAG = "pclika_bme280";

/* ── Register map ────────────────────────────────────────────────────────── */
#define BME280_REG_ID           0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_PRESS_MSB    0xF7
#define BME280_CHIP_ID          0x60
#define BME280_RESET_VAL        0xB6

/* Calibration register base */
#define BME280_CALIB_T1_L       0x88   /* 0x88..0x9F = T1..P9 */
#define BME280_CALIB_H1         0xA1
#define BME280_CALIB_H2_L       0xE1   /* 0xE1..0xE7 = H2..H6 */

#define I2C_PORT            I2C_NUM_0
#define I2C_FREQ_HZ         400000
#define I2C_TIMEOUT_MS      100
#define I2C_ACK_CHECK_EN    true

/* ── State ───────────────────────────────────────────────────────────────── */
typedef struct {
    uint8_t addr;
    bool    initialized;

    /* Compensation coefficients */
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4, dig_H5;
    int8_t   dig_H6;
} bme280_state_t;

static bme280_state_t s_bme = {0};

/* ── I2C helpers ─────────────────────────────────────────────────────────── */
static esp_err_t bme280_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_write_to_device(I2C_PORT, s_bme.addr,
                                      buf, 2,
                                      pdMS_TO_TICKS(I2C_TIMEOUT_MS));
}

static esp_err_t bme280_read_regs(uint8_t reg, uint8_t *dst, size_t len)
{
    return i2c_master_write_read_device(I2C_PORT, s_bme.addr,
                                        &reg, 1,
                                        dst, len,
                                        pdMS_TO_TICKS(I2C_TIMEOUT_MS));
}

/* ── Calibration load ────────────────────────────────────────────────────── */
static esp_err_t bme280_read_calibration(void)
{
    uint8_t calib[26];
    ESP_RETURN_ON_ERROR(
        bme280_read_regs(BME280_CALIB_T1_L, calib, 24),
        TAG, "calib T read failed");

    s_bme.dig_T1 = (uint16_t)(calib[1]  << 8 | calib[0]);
    s_bme.dig_T2 = (int16_t) (calib[3]  << 8 | calib[2]);
    s_bme.dig_T3 = (int16_t) (calib[5]  << 8 | calib[4]);
    s_bme.dig_P1 = (uint16_t)(calib[7]  << 8 | calib[6]);
    s_bme.dig_P2 = (int16_t) (calib[9]  << 8 | calib[8]);
    s_bme.dig_P3 = (int16_t) (calib[11] << 8 | calib[10]);
    s_bme.dig_P4 = (int16_t) (calib[13] << 8 | calib[12]);
    s_bme.dig_P5 = (int16_t) (calib[15] << 8 | calib[14]);
    s_bme.dig_P6 = (int16_t) (calib[17] << 8 | calib[16]);
    s_bme.dig_P7 = (int16_t) (calib[19] << 8 | calib[18]);
    s_bme.dig_P8 = (int16_t) (calib[21] << 8 | calib[20]);
    s_bme.dig_P9 = (int16_t) (calib[23] << 8 | calib[22]);

    uint8_t h1;
    ESP_RETURN_ON_ERROR(
        bme280_read_regs(BME280_CALIB_H1, &h1, 1),
        TAG, "calib H1 read failed");
    s_bme.dig_H1 = h1;

    uint8_t hcal[7];
    ESP_RETURN_ON_ERROR(
        bme280_read_regs(BME280_CALIB_H2_L, hcal, 7),
        TAG, "calib H2-H6 read failed");
    s_bme.dig_H2 = (int16_t)(hcal[1] << 8 | hcal[0]);
    s_bme.dig_H3 = hcal[2];
    s_bme.dig_H4 = (int16_t)((hcal[3] << 4) | (hcal[4] & 0x0F));
    s_bme.dig_H5 = (int16_t)((hcal[5] << 4) | (hcal[4] >> 4));
    s_bme.dig_H6 = (int8_t)hcal[6];

    return ESP_OK;
}

/* ── Bosch compensation formulas (32-bit integer) ────────────────────────── */
static int32_t t_fine;  // shared between T and H compensation

static float bme280_compensate_T(int32_t adc_T)
{
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)s_bme.dig_T1 << 1))) *
                    ((int32_t)s_bme.dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - (int32_t)s_bme.dig_T1) *
                       ((adc_T >> 4) - (int32_t)s_bme.dig_T1)) >> 12) *
                    ((int32_t)s_bme.dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (float)((t_fine * 5 + 128) >> 8) / 100.0f;
}

static float bme280_compensate_P(int32_t adc_P)
{
    int64_t var1 = ((int64_t)t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)s_bme.dig_P6;
    var2 += ((var1 * (int64_t)s_bme.dig_P5) << 17);
    var2 += ((int64_t)s_bme.dig_P4 << 35);
    var1  = ((var1 * var1 * (int64_t)s_bme.dig_P3) >> 8) +
            ((var1 * (int64_t)s_bme.dig_P2) << 12);
    var1  = (((((int64_t)1) << 47) + var1) * (int64_t)s_bme.dig_P1) >> 33;
    if (var1 == 0) return 0.0f;
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)s_bme.dig_P9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = ((int64_t)s_bme.dig_P8 * p) >> 19;
    p = ((p + var1 + var2) >> 8) + ((int64_t)s_bme.dig_P7 << 4);
    return (float)p / 25600.0f;  /* Pa */
}

static float bme280_compensate_H(int32_t adc_H)
{
    int32_t v = t_fine - 76800;
    v = (((((adc_H << 14) - ((int32_t)s_bme.dig_H4 << 20) -
            ((int32_t)s_bme.dig_H5 * v)) + 16384) >> 15) *
         (((((((v * (int32_t)s_bme.dig_H6) >> 10) *
              (((v * (int32_t)s_bme.dig_H3) >> 11) + 32768)) >> 10) +
            2097152) * (int32_t)s_bme.dig_H2 + 8192) >> 14));
    v = v - (((((v >> 15) * (v >> 15)) >> 7) * (int32_t)s_bme.dig_H1) >> 4);
    v = (v < 0) ? 0 : v;
    v = (v > 419430400) ? 419430400 : v;
    return (float)(v >> 12) / 1024.0f;
}

/* ── Public API ──────────────────────────────────────────────────────────── */
esp_err_t pclika_bme280_init(int sda_gpio, int scl_gpio, uint8_t i2c_addr)
{
    if (s_bme.initialized) return ESP_OK;

    s_bme.addr = i2c_addr;

    /* I2C init */
    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = sda_gpio,
        .scl_io_num       = scl_gpio,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(I2C_PORT, &conf),
                        TAG, "i2c_param_config failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0),
                        TAG, "i2c_driver_install failed");

    /* Verify chip ID */
    uint8_t chip_id = 0;
    ESP_RETURN_ON_ERROR(bme280_read_regs(BME280_REG_ID, &chip_id, 1),
                        TAG, "chip_id read failed");
    if (chip_id != BME280_CHIP_ID) {
        ESP_LOGE(TAG, "Wrong chip ID: 0x%02X (expected 0x%02X)", chip_id, BME280_CHIP_ID);
        return ESP_ERR_NOT_FOUND;
    }

    /* Soft reset */
    bme280_write_reg(BME280_REG_RESET, BME280_RESET_VAL);
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Load calibration */
    ESP_RETURN_ON_ERROR(bme280_read_calibration(), TAG, "calibration failed");

    /* Configure: osrs_h=x1, osrs_t=x1, osrs_p=x1, mode=normal, t_standby=1000ms */
    bme280_write_reg(BME280_REG_CTRL_HUM,  0x01);   // humidity osrs_h=x1
    bme280_write_reg(BME280_REG_CONFIG,    0xA0);   // t_sb=1000ms, filter=off
    bme280_write_reg(BME280_REG_CTRL_MEAS, 0x27);   // osrs_t=x1, osrs_p=x1, mode=normal

    s_bme.initialized = true;
    ESP_LOGI(TAG, "BME280 initialized at 0x%02X (chip_id=0x%02X)", i2c_addr, chip_id);
    return ESP_OK;
}

esp_err_t pclika_bme280_read(pclika_bme280_data_t *out)
{
    if (!s_bme.initialized) return ESP_ERR_INVALID_STATE;
    if (!out)               return ESP_ERR_INVALID_ARG;

    /* Check measurement status */
    uint8_t status = 0;
    bme280_read_regs(BME280_REG_STATUS, &status, 1);
    if (status & 0x08) {
        /* Measurement in progress — wait up to 100 ms */
        int retries = 10;
        while ((status & 0x08) && retries-- > 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            bme280_read_regs(BME280_REG_STATUS, &status, 1);
        }
        if (status & 0x08) return ESP_ERR_TIMEOUT;
    }

    /* Burst read 8 bytes: press(3) + temp(3) + hum(2) */
    uint8_t raw[8];
    ESP_RETURN_ON_ERROR(
        bme280_read_regs(BME280_REG_PRESS_MSB, raw, 8),
        TAG, "raw read failed");

    int32_t adc_P = ((int32_t)raw[0] << 12) | ((int32_t)raw[1] << 4) | (raw[2] >> 4);
    int32_t adc_T = ((int32_t)raw[3] << 12) | ((int32_t)raw[4] << 4) | (raw[5] >> 4);
    int32_t adc_H = ((int32_t)raw[6] << 8)  |  (int32_t)raw[7];

    /* Must compensate T first (sets t_fine) */
    out->temperature_c = bme280_compensate_T(adc_T);
    out->pressure_pa   = bme280_compensate_P(adc_P);
    out->humidity_pct  = bme280_compensate_H(adc_H);

    return ESP_OK;
}

esp_err_t pclika_bme280_deinit(void)
{
    if (!s_bme.initialized) return ESP_OK;
    i2c_driver_delete(I2C_PORT);
    memset(&s_bme, 0, sizeof(s_bme));
    return ESP_OK;
}
