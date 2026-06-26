/**
 * @file pclika_servo.c
 * @brief Pclika servo control via LEDC (PWM)
 *
 * Standard servo PWM: 50Hz, 1ms (0°) to 2ms (180°) pulse width.
 * Pulse range: PCLIKA_SERVO_MIN_US (500µs) to PCLIKA_SERVO_MAX_US (2500µs).
 */

#include "pclika_servo.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "pclika_servo";

/* ── Config ──────────────────────────────────────────────────────────────── */

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE
#define LEDC_RESOLUTION LEDC_TIMER_14_BIT   /* 2^14 = 16384 counts */
#define LEDC_MAX_DUTY   ((1 << 14) - 1)

#define MAX_CHANNELS    4

typedef struct {
    bool            in_use;
    int             gpio;
    ledc_channel_t  channel;
    int             current_angle;
} servo_ch_t;

static servo_ch_t s_channels[MAX_CHANNELS];
static bool       s_timer_init = false;
static bool       s_initialized = false;

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static uint32_t angle_to_duty(int angle)
{
    /* Map 0–180° → PCLIKA_SERVO_MIN_US – PCLIKA_SERVO_MAX_US */
    int pulse_us = PCLIKA_SERVO_MIN_US
                 + (angle * (PCLIKA_SERVO_MAX_US - PCLIKA_SERVO_MIN_US)) / 180;
    /* duty = pulse_us / period_us * max_duty
     * period_us = 1_000_000 / PCLIKA_SERVO_FREQ_HZ = 20000 µs */
    uint32_t period_us = 1000000 / PCLIKA_SERVO_FREQ_HZ;
    return (uint32_t)pulse_us * LEDC_MAX_DUTY / period_us;
}

/* ── Init ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_servo_init(void)
{
    if (s_initialized) return ESP_OK;

    memset(s_channels, 0, sizeof(s_channels));

    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_SPEED_MODE,
        .timer_num       = LEDC_TIMER,
        .duty_resolution = LEDC_RESOLUTION,
        .freq_hz         = PCLIKA_SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    esp_err_t err = ledc_timer_config(&timer_cfg);
    if (err != ESP_OK) return err;

    s_timer_init  = true;
    s_initialized = true;
    ESP_LOGI(TAG, "servo init: %dHz, 14-bit, range %d-%dµs",
             PCLIKA_SERVO_FREQ_HZ, PCLIKA_SERVO_MIN_US, PCLIKA_SERVO_MAX_US);
    return ESP_OK;
}

/* ── Attach ──────────────────────────────────────────────────────────────── */

esp_err_t pclika_servo_attach(int channel, int gpio)
{
    if (channel < 0 || channel >= MAX_CHANNELS) return ESP_ERR_INVALID_ARG;
    if (!s_initialized) return ESP_ERR_INVALID_STATE;

    ledc_channel_config_t ch_cfg = {
        .gpio_num   = gpio,
        .speed_mode = LEDC_SPEED_MODE,
        .channel    = (ledc_channel_t)channel,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER,
        .duty       = angle_to_duty(90),  /* Start at center */
        .hpoint     = 0,
    };
    esp_err_t err = ledc_channel_config(&ch_cfg);
    if (err != ESP_OK) return err;

    s_channels[channel].in_use        = true;
    s_channels[channel].gpio          = gpio;
    s_channels[channel].channel       = (ledc_channel_t)channel;
    s_channels[channel].current_angle = 90;

    ESP_LOGI(TAG, "servo ch%d attached to GPIO%d", channel, gpio);
    return ESP_OK;
}

/* ── Move ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_servo_move(int channel, int angle, int speed_ms)
{
    if (channel < 0 || channel >= MAX_CHANNELS) return ESP_ERR_INVALID_ARG;
    if (!s_channels[channel].in_use)            return ESP_ERR_INVALID_STATE;
    if (angle < 0 || angle > 180)               return ESP_ERR_INVALID_ARG;

    int current = s_channels[channel].current_angle;

    if (speed_ms <= 0 || abs(angle - current) <= 1) {
        /* Instant move */
        uint32_t duty = angle_to_duty(angle);
        ledc_set_duty(LEDC_SPEED_MODE, s_channels[channel].channel, duty);
        ledc_update_duty(LEDC_SPEED_MODE, s_channels[channel].channel);
    } else {
        /* Sweep — 1° per step */
        int step = (angle > current) ? 1 : -1;
        int steps = abs(angle - current);
        int delay_per_step_ms = speed_ms / (steps > 0 ? steps : 1);
        if (delay_per_step_ms < 1) delay_per_step_ms = 1;

        for (int a = current; a != angle; a += step) {
            uint32_t duty = angle_to_duty(a);
            ledc_set_duty(LEDC_SPEED_MODE, s_channels[channel].channel, duty);
            ledc_update_duty(LEDC_SPEED_MODE, s_channels[channel].channel);
            vTaskDelay(pdMS_TO_TICKS(delay_per_step_ms));
        }
        /* Final position */
        uint32_t duty = angle_to_duty(angle);
        ledc_set_duty(LEDC_SPEED_MODE, s_channels[channel].channel, duty);
        ledc_update_duty(LEDC_SPEED_MODE, s_channels[channel].channel);
    }

    s_channels[channel].current_angle = angle;
    ESP_LOGI(TAG, "servo ch%d → %d°", channel, angle);
    return ESP_OK;
}

/* ── Current angle ───────────────────────────────────────────────────────── */

int pclika_servo_get_angle(int channel)
{
    if (channel < 0 || channel >= MAX_CHANNELS) return -1;
    if (!s_channels[channel].in_use)            return -1;
    return s_channels[channel].current_angle;
}
