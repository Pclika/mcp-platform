/**
 * @file pclika_display.c
 * @brief Pclika display registry — driver registration + unified text API
 */

#include "pclika_display.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "pclika_display";

#define MAX_DISPLAYS 4

static const pclika_display_desc_t *s_displays[MAX_DISPLAYS];
static int  s_count       = 0;
static bool s_initialized = false;

/* ── Init ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_display_init(void)
{
    if (s_initialized) return ESP_OK;
    memset(s_displays, 0, sizeof(s_displays));
    s_count = 0;
    s_initialized = true;
    ESP_LOGI(TAG, "display registry init");
    return ESP_OK;
}

/* ── Registration ────────────────────────────────────────────────────────── */

esp_err_t pclika_display_register(const pclika_display_desc_t *desc)
{
    if (!desc || !desc->id) return ESP_ERR_INVALID_ARG;
    if (s_count >= MAX_DISPLAYS) return ESP_ERR_NO_MEM;

    for (int i = 0; i < s_count; i++) {
        if (strcmp(s_displays[i]->id, desc->id) == 0) {
            ESP_LOGW(TAG, "display '%s' already registered — replacing", desc->id);
            s_displays[i] = desc;
            return ESP_OK;
        }
    }

    s_displays[s_count++] = desc;
    ESP_LOGI(TAG, "registered display '%s' (%dx%d)", desc->id, desc->cols, desc->rows);
    return ESP_OK;
}

/* ── Text ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_display_text(const char *id, const char *text, int line, bool clear_first)
{
    if (!id || !text) return ESP_ERR_INVALID_ARG;

    for (int i = 0; i < s_count; i++) {
        if (strcmp(s_displays[i]->id, id) == 0) {
            if (!s_displays[i]->initialized) return ESP_ERR_INVALID_STATE;
            if (!s_displays[i]->text_fn)     return ESP_ERR_NOT_SUPPORTED;
            return s_displays[i]->text_fn(s_displays[i]->ctx, text, line, clear_first);
        }
    }

    ESP_LOGW(TAG, "display '%s' not found", id);
    return ESP_ERR_NOT_FOUND;
}

/* ── Clear ───────────────────────────────────────────────────────────────── */

esp_err_t pclika_display_clear(const char *id)
{
    if (!id) return ESP_ERR_INVALID_ARG;

    for (int i = 0; i < s_count; i++) {
        if (strcmp(s_displays[i]->id, id) == 0) {
            if (!s_displays[i]->initialized) return ESP_ERR_INVALID_STATE;
            if (!s_displays[i]->clear_fn)    return ESP_ERR_NOT_SUPPORTED;
            return s_displays[i]->clear_fn(s_displays[i]->ctx);
        }
    }

    return ESP_ERR_NOT_FOUND;
}
