/**
 * @file main.c
 * @brief env-monitor — Environmental monitoring via MCP
 *
 * Hardware:
 *   - Pclika baseboard (ESP32-S3)
 *   - DHT22 temperature/humidity sensor → GPIO4
 *   - SSD1306 OLED 128×64 (I2C) → SDA=GPIO8, SCL=GPIO9
 *
 * MCP tools:
 *   - sensor_read(sensor_id="temp_humidity") → {temp, humidity}
 *   - display_text(text="...", clear=true)   → updates OLED
 *   - device_info                            → board identity
 *   - serial_log_read                        → runtime logs
 *
 * Platform seal: PCK-MMXXVI-C4A32096
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "pclika_system.h"
#include "pclika_logging.h"
#include "pclika_sensor.h"
#include "pclika_display.h"
#include "pclika_gpio.h"
#include "pclika_bridge.h"

/* ── Pin configuration ───────────────────────────────────────────────────── */
#define DHT22_GPIO      4
#define I2C_SDA_GPIO    8
#define I2C_SCL_GPIO    9
#define I2C_PORT        I2C_NUM_0
#define OLED_I2C_ADDR   0x3C

static const char *TAG = "env_monitor";

/* Platform seal */
const char PCLIKA_ORIGIN_SEAL[] __attribute__((used, section(".rodata.pclika"))) =
    "PCK:ORIGIN:2026-06-25:c4a32096:env-monitor-v0.1.0";

/* ── DHT22 sensor driver (minimal implementation) ────────────────────────── */
#include "driver/gpio.h"
#include "esp_timer.h"
#include <math.h>

static float s_last_temp     = NAN;
static float s_last_humidity = NAN;
static int64_t s_last_read_us = 0;

#define DHT22_MIN_INTERVAL_US  (2000000)  /* 2 seconds between reads */

static esp_err_t dht22_read_raw(int gpio, float *temp_out, float *hum_out)
{
    /* DHT22 1-wire protocol — simplified polling implementation */
    uint8_t data[5] = {0};

    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(gpio, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(gpio, GPIO_MODE_INPUT);

    /* Wait for DHT response pulse */
    int64_t t = esp_timer_get_time();
    while (gpio_get_level(gpio) == 1) {
        if (esp_timer_get_time() - t > 100) return ESP_ERR_TIMEOUT;
    }
    while (gpio_get_level(gpio) == 0) {
        if (esp_timer_get_time() - t > 200) return ESP_ERR_TIMEOUT;
    }
    while (gpio_get_level(gpio) == 1) {
        if (esp_timer_get_time() - t > 300) return ESP_ERR_TIMEOUT;
    }

    /* Read 40 bits */
    for (int i = 0; i < 40; i++) {
        while (gpio_get_level(gpio) == 0);
        int64_t t_high = esp_timer_get_time();
        while (gpio_get_level(gpio) == 1);
        int64_t dt = esp_timer_get_time() - t_high;
        data[i / 8] <<= 1;
        if (dt > 40) data[i / 8] |= 1;  /* >40µs = '1' bit */
    }

    /* Checksum */
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        return ESP_ERR_INVALID_CRC;
    }

    *hum_out  = ((data[0] << 8) | data[1]) * 0.1f;
    *temp_out = (((data[2] & 0x7F) << 8) | data[3]) * 0.1f;
    if (data[2] & 0x80) *temp_out = -(*temp_out);

    return ESP_OK;
}

static esp_err_t dht22_sensor_read_fn(int channel, pclika_sensor_reading_t *out)
{
    (void)channel;
    int64_t now = esp_timer_get_time();

    /* Throttle reads — DHT22 needs at least 2s between reads */
    if (!isnan(s_last_temp) && (now - s_last_read_us) < DHT22_MIN_INTERVAL_US) {
        out->value      = s_last_temp;
        out->value2     = s_last_humidity;
        out->has_value2 = true;
        out->valid      = true;
        snprintf(out->unit,  sizeof(out->unit),  "celsius");
        snprintf(out->unit2, sizeof(out->unit2), "%%RH");
        out->timestamp_us = s_last_read_us;
        return ESP_OK;
    }

    float temp, hum;
    esp_err_t err = dht22_read_raw(DHT22_GPIO, &temp, &hum);
    if (err != ESP_OK) {
        out->valid = false;
        snprintf(out->error, sizeof(out->error), "DHT22 read failed (%s)", esp_err_to_name(err));
        return err;
    }

    s_last_temp     = temp;
    s_last_humidity = hum;
    s_last_read_us  = now;

    out->value      = temp;
    out->value2     = hum;
    out->has_value2 = true;
    out->valid      = true;
    out->timestamp_us = now;
    snprintf(out->unit,  sizeof(out->unit),  "celsius");
    snprintf(out->unit2, sizeof(out->unit2), "%%RH");
    return ESP_OK;
}

static const pclika_sensor_desc_t DHT22_DESC = {
    .id            = "temp_humidity",
    .read_fn       = dht22_sensor_read_fn,
    .channel_count = 1,
    .initialized   = true,
};

/* ── OLED driver stub ────────────────────────────────────────────────────── */
/*
 * Full SSD1306 I2C driver implementation would go here.
 * For now, we use a stub that logs text — replace with actual driver.
 */
static esp_err_t oled_clear_fn(void *ctx) {
    ESP_LOGI(TAG, "[OLED] clear");
    return ESP_OK;
}

static esp_err_t oled_text_fn(void *ctx, const char *text, int line, bool clear_first) {
    if (clear_first) ESP_LOGI(TAG, "[OLED] clear");
    ESP_LOGI(TAG, "[OLED] line %d: %s", line, text);
    return ESP_OK;
}

static const pclika_display_desc_t OLED_DESC = {
    .id          = "primary",
    .ctx         = NULL,
    .clear_fn    = oled_clear_fn,
    .text_fn     = oled_text_fn,
    .cols        = 16,
    .rows        = 4,
    .initialized = true,
};

/* ── Auto-display task ───────────────────────────────────────────────────── */

static void env_display_task(void *arg)
{
    char line0[32], line1[32];
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));

        pclika_sensor_reading_t r = {0};
        if (pclika_sensor_read("temp_humidity", 0, &r) == ESP_OK && r.valid) {
            snprintf(line0, sizeof(line0), "Temp: %.1f C",   r.value);
            snprintf(line1, sizeof(line1), "Hum:  %.1f %%", r.value2);
            pclika_display_text("primary", line0, 0, true);
            pclika_display_text("primary", line1, 1, false);
            pclika_log_info("env: %.1f°C  %.1f%%RH", r.value, r.value2);
        } else {
            pclika_log_warn("env: sensor read failed");
        }
    }
}

/* ── app_main ────────────────────────────────────────────────────────────── */

void app_main(void)
{
    nvs_flash_init();
    pclika_logging_init();

    pclika_log_info("=== env-monitor v0.1.0 ===");
    pclika_log_info("Seal: PCK-MMXXVI-C4A32096");
    pclika_log_info("DHT22 → GPIO%d", DHT22_GPIO);
    pclika_log_info("SSD1306 OLED → SDA=GPIO%d SCL=GPIO%d", I2C_SDA_GPIO, I2C_SCL_GPIO);

    pclika_system_init();
    pclika_gpio_init();

    /* Sensor init + DHT22 registration */
    pclika_sensor_init();
    pclika_sensor_register(&DHT22_DESC);
    pclika_system_set_cap("has_sensor", true);

    /* Display init + OLED registration */
    pclika_display_init();
    pclika_display_register(&OLED_DESC);
    pclika_system_set_cap("has_display", true);

    /* Show startup message on display */
    pclika_display_text("primary", "Pclika", 0, true);
    pclika_display_text("primary", "env-monitor", 1, false);
    pclika_display_text("primary", "v0.1.0", 2, false);

    /* Auto-refresh display every 5 seconds */
    xTaskCreate(env_display_task, "env_display", 4096, NULL, 3, NULL);

    /* Start MCP bridge */
    pclika_bridge_config_t cfg = PCLIKA_BRIDGE_CONFIG_DEFAULT();
    pclika_bridge_start(&cfg);

    pclika_log_info("env-monitor ready");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}
