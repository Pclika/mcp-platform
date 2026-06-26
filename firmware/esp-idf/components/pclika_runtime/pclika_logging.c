/**
 * @file pclika_logging.c
 * @brief Ring-buffer log capture for MCP serial_log_read tool
 */

#include "pclika_logging.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static const char *TAG = "pclika_log";

/* ── Ring buffer ─────────────────────────────────────────────────────────── */

typedef struct {
    char lines[PCLIKA_LOG_BUF_LINES][PCLIKA_LOG_LINE_MAX];
    int  head;
    int  count;
} log_ring_t;

static log_ring_t s_ring;
static SemaphoreHandle_t s_mutex;
static bool s_initialized = false;

/* ── vprintf hook ────────────────────────────────────────────────────────── */

static int pclika_log_vprintf(const char *fmt, va_list args)
{
    /* Write to normal UART */
    int ret = vprintf(fmt, args);

    /* Also capture into ring buffer */
    if (s_initialized && s_mutex) {
        char buf[PCLIKA_LOG_LINE_MAX];
        va_list args2;
        va_copy(args2, args);
        vsnprintf(buf, sizeof(buf), fmt, args2);
        va_end(args2);

        /* Strip trailing newline */
        int len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';

        if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            int idx = (s_ring.head + s_ring.count) % PCLIKA_LOG_BUF_LINES;
            if (s_ring.count >= PCLIKA_LOG_BUF_LINES) {
                /* Overwrite oldest */
                s_ring.head = (s_ring.head + 1) % PCLIKA_LOG_BUF_LINES;
            } else {
                s_ring.count++;
            }
            snprintf(s_ring.lines[idx], PCLIKA_LOG_LINE_MAX, "%s", buf);
            xSemaphoreGive(s_mutex);
        }
    }
    return ret;
}

/* ── Public API ──────────────────────────────────────────────────────────── */

esp_err_t pclika_logging_init(void)
{
    if (s_initialized) return ESP_OK;

    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) return ESP_ERR_NO_MEM;

    memset(&s_ring, 0, sizeof(s_ring));
    s_initialized = true;

    esp_log_set_vprintf(pclika_log_vprintf);
    ESP_LOGI(TAG, "log capture init (%d lines x %d bytes)",
             PCLIKA_LOG_BUF_LINES, PCLIKA_LOG_LINE_MAX);
    return ESP_OK;
}

int pclika_log_get_lines(char out[][PCLIKA_LOG_LINE_MAX], int max_lines)
{
    if (!s_initialized || !out || max_lines <= 0) return 0;

    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(50)) != pdTRUE) return 0;

    int n = (max_lines < s_ring.count) ? max_lines : s_ring.count;
    /* Return the last n lines (newest) */
    int start = (s_ring.head + s_ring.count - n + PCLIKA_LOG_BUF_LINES) % PCLIKA_LOG_BUF_LINES;
    for (int i = 0; i < n; i++) {
        int idx = (start + i) % PCLIKA_LOG_BUF_LINES;
        snprintf(out[i], PCLIKA_LOG_LINE_MAX, "%s", s_ring.lines[idx]);
    }

    xSemaphoreGive(s_mutex);
    return n;
}

void pclika_log_info(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ESP_LOGI("pclika", "%s", buf);
}

void pclika_log_warn(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ESP_LOGW("pclika", "%s", buf);
}

void pclika_log_error(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ESP_LOGE("pclika", "%s", buf);
}
