#pragma once
/**
 * @file pclika_gpio.h
 * @brief Pclika Runtime — GPIO module
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PCLIKA_PULL_NONE = 0,
    PCLIKA_PULL_UP,
    PCLIKA_PULL_DOWN,
} pclika_pull_t;

/** Initialize GPIO subsystem. */
esp_err_t pclika_gpio_init(void);

/**
 * @brief Read a GPIO pin digital level.
 * @param pin   GPIO number
 * @param pull  Pull resistor configuration
 * @param[out] level  0 or 1
 */
esp_err_t pclika_gpio_read(int pin, pclika_pull_t pull, int *level);

/**
 * @brief Write a GPIO pin digital level.
 * @param pin   GPIO number
 * @param level 0 (LOW) or 1 (HIGH)
 */
esp_err_t pclika_gpio_write(int pin, int level);

/** Serialize gpio_read result to JSON. */
int pclika_gpio_read_to_json(int pin, int level, char *buf, size_t len);

/** Serialize gpio_write result to JSON. */
int pclika_gpio_write_to_json(int pin, int level, bool ok, char *buf, size_t len);

#ifdef __cplusplus
}
#endif
