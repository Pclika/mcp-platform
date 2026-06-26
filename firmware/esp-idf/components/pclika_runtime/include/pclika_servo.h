#pragma once
/**
 * @file pclika_servo.h
 * @brief Pclika Runtime — Servo / PWM motion module
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/ledc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_SERVO_MAX_CHANNELS   16
#define PCLIKA_SERVO_FREQ_HZ        50       /**< Standard servo PWM frequency */
#define PCLIKA_SERVO_MIN_US         500      /**< Pulse width for 0°   */
#define PCLIKA_SERVO_MAX_US         2500     /**< Pulse width for 180° */

typedef struct {
    int         gpio_pin;
    ledc_channel_t ledc_channel;
    float       current_angle;
    bool        initialized;
} pclika_servo_channel_t;

/** Initialize servo subsystem. Configures LEDC timer. */
esp_err_t pclika_servo_init(void);

/**
 * @brief Configure a servo channel on a GPIO pin.
 * @param channel  Channel index (0–15)
 * @param gpio_pin GPIO pin number
 */
esp_err_t pclika_servo_config_channel(int channel, int gpio_pin);

/**
 * @brief Move servo to angle with optional speed ramping.
 * @param channel  Channel index
 * @param angle    Target angle (0.0–180.0 degrees)
 * @param speed    1–100; 100 = immediate, lower = slower ramp
 */
esp_err_t pclika_servo_move(int channel, float angle, int speed);

/** Get current angle of a channel. */
float pclika_servo_get_angle(int channel);

/** Serialize move result to JSON. Returns bytes written. */
int pclika_servo_result_to_json(int channel, float angle, bool ok,
                                 const char *error, char *buf, size_t len);

#ifdef __cplusplus
}
#endif
