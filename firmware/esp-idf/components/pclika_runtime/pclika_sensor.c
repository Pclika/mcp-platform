/**
 * @file pclika_sensor.c
 * @brief Pclika sensor registry — driver registration + unified read API
 */

#include "pclika_sensor.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "pclika_sensor";

#define MAX_SENSORS 8

static const pclika_sensor_desc_t *s_sensors[MAX_SENSORS];
static int   s_count       = 0;
static bool  s_initialized = false;

/* ── Init ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_sensor_init(void)
{
    if (s_initialized) return ESP_OK;
    memset(s_sensors, 0, sizeof(s_sensors));
    s_count = 0;
    s_initialized = true;
    ESP_LOGI(TAG, "sensor registry init");
    return ESP_OK;
}

/* ── Registration ────────────────────────────────────────────────────────── */

esp_err_t pclika_sensor_register(const pclika_sensor_desc_t *desc)
{
    if (!desc || !desc->id || !desc->read_fn) return ESP_ERR_INVALID_ARG;
    if (s_count >= MAX_SENSORS) return ESP_ERR_NO_MEM;

    /* Duplicate check */
    for (int i = 0; i < s_count; i++) {
        if (strcmp(s_sensors[i]->id, desc->id) == 0) {
            ESP_LOGW(TAG, "sensor '%s' already registered — replacing", desc->id);
            s_sensors[i] = desc;
            return ESP_OK;
        }
    }

    s_sensors[s_count++] = desc;
    ESP_LOGI(TAG, "registered sensor '%s' (channels=%d)", desc->id, desc->channel_count);
    return ESP_OK;
}

/* ── Read ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_sensor_read(const char *id, int channel, pclika_sensor_reading_t *out)
{
    if (!id || !out) return ESP_ERR_INVALID_ARG;
    memset(out, 0, sizeof(*out));

    for (int i = 0; i < s_count; i++) {
        if (strcmp(s_sensors[i]->id, id) == 0) {
            if (!s_sensors[i]->initialized) {
                snprintf(out->error, sizeof(out->error), "sensor '%s' not initialized", id);
                out->valid = false;
                return ESP_ERR_INVALID_STATE;
            }
            return s_sensors[i]->read_fn(channel, out);
        }
    }

    snprintf(out->error, sizeof(out->error), "sensor '%s' not found", id);
    out->valid = false;
    ESP_LOGW(TAG, "sensor '%s' not found", id);
    return ESP_ERR_NOT_FOUND;
}

/* ── List ────────────────────────────────────────────────────────────────── */

int pclika_sensor_list(const char **ids_out, int max_count)
{
    int n = (max_count < s_count) ? max_count : s_count;
    for (int i = 0; i < n; i++) {
        ids_out[i] = s_sensors[i]->id;
    }
    return n;
}
