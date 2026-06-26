/**
 * servo-control/main.c — SG90 Servo Control via MCP Bridge
 *
 * Demonstrates:
 *   - pclika_servo component (LEDC PWM, 50 Hz)
 *   - FreeRTOS servo sweep task
 *   - NDJSON MCP bridge (MCP tool: servo_move)
 *
 * Servo on LEDC CH0, GPIO 5.
 * Serial bridge: 921600 baud, USB-Serial.
 *
 * ORIGIN_SEAL: "PCK:ORIGIN:2026-06-25:c4a32096:servo-control-v0.1.0"
 * Seal: PCK-MMXXVI-C4A32096
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "pclika_runtime.h"
#include "pclika_servo.h"
#include "pclika_bridge.h"

static const char *TAG = "servo-control";

/* ── Servo configuration ─────────────────────────────────────────────────── */
#define SERVO_CHANNEL   0
#define SERVO_GPIO      5

/* ── MCP tool: servo_move ─────────────────────────────────────────────────── */
static cJSON *tool_servo_move(cJSON *params, void *ctx)
{
    int channel  = cJSON_GetObjectItem(params, "channel")  ?
                   cJSON_GetObjectItem(params, "channel")->valueint  : 0;
    int angle    = cJSON_GetObjectItem(params, "angle")    ?
                   cJSON_GetObjectItem(params, "angle")->valueint    : 90;
    int speed_ms = cJSON_GetObjectItem(params, "speed_ms") ?
                   cJSON_GetObjectItem(params, "speed_ms")->valueint : 0;

    esp_err_t err = pclika_servo_move(channel, angle, speed_ms);

    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "status", err == ESP_OK ? "ok" : "error");
    cJSON_AddNumberToObject(result, "channel", channel);
    cJSON_AddNumberToObject(result, "angle",   angle);
    if (err != ESP_OK) {
        cJSON_AddStringToObject(result, "error", esp_err_to_name(err));
    }
    return result;
}

/* ── MCP tool: servo_read ─────────────────────────────────────────────────── */
static cJSON *tool_servo_read(cJSON *params, void *ctx)
{
    int channel = cJSON_GetObjectItem(params, "channel") ?
                  cJSON_GetObjectItem(params, "channel")->valueint : 0;

    pclika_servo_state_t state;
    esp_err_t err = pclika_servo_get_state(channel, &state);

    cJSON *result = cJSON_CreateObject();
    if (err == ESP_OK) {
        cJSON_AddNumberToObject(result, "channel",     channel);
        cJSON_AddNumberToObject(result, "angle",       state.angle);
        cJSON_AddNumberToObject(result, "target_angle", state.target_angle);
        cJSON_AddBoolToObject  (result, "moving",      state.moving);
        cJSON_AddStringToObject(result, "status",      "ok");
    } else {
        cJSON_AddStringToObject(result, "status", "error");
        cJSON_AddStringToObject(result, "error",  esp_err_to_name(err));
    }
    return result;
}

/* ── Sweep task ──────────────────────────────────────────────────────────── */
static void servo_sweep_task(void *pvParam)
{
    ESP_LOGI(TAG, "Servo sweep task started");
    int angles[]  = {0, 45, 90, 135, 180, 135, 90, 45};
    int n_angles  = sizeof(angles) / sizeof(angles[0]);
    int i = 0;

    while (1) {
        int angle = angles[i % n_angles];
        ESP_LOGI(TAG, "Sweep → %d°", angle);
        pclika_servo_move(SERVO_CHANNEL, angle, 300);
        i++;
        vTaskDelay(pdMS_TO_TICKS(700));
    }
}

/* ── app_main ─────────────────────────────────────────────────────────────── */
void app_main(void)
{
    ESP_LOGI(TAG, "Pclika servo-control v0.1.0");
    ESP_LOGI(TAG, "ORIGIN_SEAL: PCK:ORIGIN:2026-06-25:c4a32096:servo-control-v0.1.0");

    /* Runtime init */
    pclika_runtime_init();

    /* Servo init — channel 0, GPIO 5, 50 Hz */
    pclika_servo_config_t servo_cfg = {
        .channel        = SERVO_CHANNEL,
        .gpio_num       = SERVO_GPIO,
        .min_pulse_us   = 500,    // SG90: 0.5 ms → 0°
        .max_pulse_us   = 2500,   // SG90: 2.5 ms → 180°
        .pwm_freq_hz    = 50,
    };
    ESP_ERROR_CHECK(pclika_servo_init(&servo_cfg));
    ESP_LOGI(TAG, "Servo initialized: CH%d GPIO%d", SERVO_CHANNEL, SERVO_GPIO);

    /* Register capabilities */
    pclika_runtime_register_cap("servo_control");
    pclika_runtime_register_cap("pwm_50hz");

    /* Register MCP tools */
    pclika_bridge_register_tool("servo_move", tool_servo_move, NULL);
    pclika_bridge_register_tool("servo_read", tool_servo_read, NULL);

    /* Start sweep task */
    xTaskCreate(servo_sweep_task, "servo_sweep", 4096, NULL, 5, NULL);

    /* Start MCP bridge (blocking — reads NDJSON from UART) */
    pclika_bridge_run();
}
