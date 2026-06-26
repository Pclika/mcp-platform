/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * vision-snapshot — MCP Example
 * PCK-MMXXVI-C4A32096
 *
 * Captures JPEG frames from OV2640 on ESP32-S3 and exposes them via MCP.
 * Requires ESP32-S3 CAM board (e.g., Seeed XIAO-S3 Sense, AI-Thinker ESP32-CAM).
 *
 * MCP tools exposed: camera_capture, camera_status, camera_configure
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mbedtls/base64.h"
#include "cJSON.h"

#include "pclika_bridge.h"
#include "pclika_runtime.h"

#define TAG "vision-snapshot"

/* -------- Camera pin config — XIAO-S3 Sense -------- */
/* Override with sdkconfig for other boards            */
#ifndef CAM_PIN_PWDN
#define CAM_PIN_PWDN   -1
#define CAM_PIN_RESET  -1
#define CAM_PIN_XCLK    10
#define CAM_PIN_SIOD     4
#define CAM_PIN_SIOC     5
#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2       8
#define CAM_PIN_D1       9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC    6
#define CAM_PIN_HREF     7
#define CAM_PIN_PCLK    13
#endif

static bool s_camera_ready = false;

/* Resolution name → enum */
static framesize_t str_to_framesize(const char *s)
{
    if (!s) return FRAMESIZE_VGA;
    if (strcmp(s, "QVGA")  == 0) return FRAMESIZE_QVGA;
    if (strcmp(s, "VGA")   == 0) return FRAMESIZE_VGA;
    if (strcmp(s, "SVGA")  == 0) return FRAMESIZE_SVGA;
    if (strcmp(s, "XGA")   == 0) return FRAMESIZE_XGA;
    if (strcmp(s, "HD")    == 0) return FRAMESIZE_HD;
    if (strcmp(s, "SXGA")  == 0) return FRAMESIZE_SXGA;
    if (strcmp(s, "UXGA")  == 0) return FRAMESIZE_UXGA;
    return FRAMESIZE_VGA;
}

static const char *framesize_to_str(framesize_t fs)
{
    switch (fs) {
        case FRAMESIZE_QVGA: return "QVGA";
        case FRAMESIZE_VGA:  return "VGA";
        case FRAMESIZE_SVGA: return "SVGA";
        case FRAMESIZE_XGA:  return "XGA";
        case FRAMESIZE_HD:   return "HD";
        case FRAMESIZE_SXGA: return "SXGA";
        case FRAMESIZE_UXGA: return "UXGA";
        default:             return "UNKNOWN";
    }
}

/* -------- Camera init -------- */

static esp_err_t camera_init(framesize_t initial_size)
{
    camera_config_t config = {
        .pin_pwdn     = CAM_PIN_PWDN,
        .pin_reset    = CAM_PIN_RESET,
        .pin_xclk     = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,
        .pin_d7 = CAM_PIN_D7, .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5, .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3, .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1, .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href  = CAM_PIN_HREF,
        .pin_pclk  = CAM_PIN_PCLK,
        .xclk_freq_hz = 20000000,
        .ledc_timer   = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size   = initial_size,
        .jpeg_quality = 20,
        .fb_count     = 2,
        .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
    };
    return esp_camera_init(&config);
}

/* -------- MCP tool handlers -------- */

static void tool_camera_capture(const cJSON *params, cJSON *result)
{
    if (!s_camera_ready) {
        cJSON_AddBoolToObject(result, "ok", false);
        cJSON_AddStringToObject(result, "error", "camera not initialized");
        return;
    }

    /* Apply per-capture params */
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor) {
        cJSON *j;
        j = cJSON_GetObjectItem(params, "resolution");
        if (cJSON_IsString(j)) sensor->set_framesize(sensor, str_to_framesize(j->valuestring));
        j = cJSON_GetObjectItem(params, "quality");
        if (cJSON_IsNumber(j)) sensor->set_quality(sensor, j->valueint);
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        cJSON_AddBoolToObject(result, "ok", false);
        cJSON_AddStringToObject(result, "error", "capture failed");
        return;
    }

    /* Base64-encode */
    size_t b64_len = 0;
    mbedtls_base64_encode(NULL, 0, &b64_len, fb->buf, fb->len);
    char *b64 = malloc(b64_len + 1);
    if (!b64) {
        esp_camera_fb_return(fb);
        cJSON_AddBoolToObject(result, "ok", false);
        cJSON_AddStringToObject(result, "error", "out of memory");
        return;
    }
    mbedtls_base64_encode((unsigned char *)b64, b64_len + 1, &b64_len,
                           fb->buf, fb->len);
    b64[b64_len] = '\0';

    cJSON_AddBoolToObject(result, "ok",          true);
    cJSON_AddNumberToObject(result, "width",      fb->width);
    cJSON_AddNumberToObject(result, "height",     fb->height);
    cJSON_AddNumberToObject(result, "size_bytes", fb->len);
    cJSON_AddStringToObject(result, "base64",     b64);
    cJSON_AddNumberToObject(result, "timestamp_ms",
                            (double)(esp_timer_get_time() / 1000));

    ESP_LOGI(TAG, "captured %dx%d %zu bytes", fb->width, fb->height, fb->len);
    free(b64);
    esp_camera_fb_return(fb);
}

static void tool_camera_status(const cJSON *params, cJSON *result)
{
    (void)params;
    cJSON_AddBoolToObject(result, "initialized", s_camera_ready);
    if (!s_camera_ready) return;

    sensor_t *sensor = esp_camera_sensor_get();
    if (!sensor) return;

    char pid_str[12];
    snprintf(pid_str, sizeof(pid_str), "0x%04X", sensor->id.PID);
    cJSON_AddStringToObject(result, "sensor_model",
                            sensor->id.PID == 0x2640 ? "OV2640" :
                            sensor->id.PID == 0x5640 ? "OV5640" : pid_str);

    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) {
        cJSON_AddStringToObject(result, "resolution",
                                framesize_to_str(sensor->status.framesize));
        cJSON_AddNumberToObject(result, "quality",    sensor->status.quality);
        cJSON_AddNumberToObject(result, "brightness", sensor->status.brightness);
        cJSON_AddNumberToObject(result, "saturation", sensor->status.saturation);
        cJSON_AddBoolToObject(result, "flip_vertical",   sensor->status.vflip);
        cJSON_AddBoolToObject(result, "flip_horizontal", sensor->status.hmirror);
        esp_camera_fb_return(fb);
    }
}

static void tool_camera_configure(const cJSON *params, cJSON *result)
{
    if (!s_camera_ready) {
        cJSON_AddBoolToObject(result, "ok", false);
        return;
    }
    sensor_t *sensor = esp_camera_sensor_get();
    if (!sensor) {
        cJSON_AddBoolToObject(result, "ok", false);
        return;
    }

    cJSON *applied = cJSON_CreateObject();
    cJSON *j;

#define APPLY_INT(key, setter) \
    j = cJSON_GetObjectItem(params, key); \
    if (cJSON_IsNumber(j)) { sensor->setter(sensor, j->valueint); \
                             cJSON_AddNumberToObject(applied, key, j->valueint); }
#define APPLY_BOOL(key, setter) \
    j = cJSON_GetObjectItem(params, key); \
    if (cJSON_IsBool(j)) { sensor->setter(sensor, cJSON_IsTrue(j) ? 1 : 0); \
                           cJSON_AddBoolToObject(applied, key, cJSON_IsTrue(j)); }

    j = cJSON_GetObjectItem(params, "resolution");
    if (cJSON_IsString(j)) {
        sensor->set_framesize(sensor, str_to_framesize(j->valuestring));
        cJSON_AddStringToObject(applied, "resolution", j->valuestring);
    }
    APPLY_INT("quality",    set_quality);
    APPLY_INT("brightness", set_brightness);
    APPLY_INT("contrast",   set_contrast);
    APPLY_INT("saturation", set_saturation);
    APPLY_BOOL("awb",             set_whitebal);
    APPLY_BOOL("aec",             set_exposure_ctrl);
    APPLY_BOOL("flip_vertical",   set_vflip);
    APPLY_BOOL("flip_horizontal", set_hmirror);

#undef APPLY_INT
#undef APPLY_BOOL

    cJSON_AddBoolToObject(result, "ok", true);
    cJSON_AddItemToObject(result, "applied", applied);
}

/* -------- app_main -------- */

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    pclika_runtime_config_t rt_cfg = PCLIKA_RUNTIME_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(pclika_runtime_init(&rt_cfg));

    esp_err_t ret = camera_init(FRAMESIZE_VGA);
    if (ret == ESP_OK) {
        s_camera_ready = true;
        ESP_LOGI(TAG, "camera initialized");
    } else {
        ESP_LOGW(TAG, "camera init failed (%s) — running without camera", esp_err_to_name(ret));
    }

    pclika_bridge_register_tool("camera_capture",   tool_camera_capture);
    pclika_bridge_register_tool("camera_status",    tool_camera_status);
    pclika_bridge_register_tool("camera_configure", tool_camera_configure);

    ESP_LOGI(TAG, "vision-snapshot ready — MCP tools: camera_capture / camera_status / camera_configure");
    pclika_bridge_run();
}
