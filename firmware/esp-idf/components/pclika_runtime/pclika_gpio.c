/**
 * @file pclika_gpio.c
 * @brief Pclika GPIO wrapper — read/write with pull configuration
 */

#include "pclika_gpio.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "pclika_gpio";

/* Pins reserved by the platform (bridge, USB, etc.) */
static const int RESERVED_PINS[] = {19, 20, 43, 44, -1};

static bool is_reserved(int pin)
{
    for (int i = 0; RESERVED_PINS[i] != -1; i++) {
        if (RESERVED_PINS[i] == pin) return true;
    }
    return false;
}

/* ── Init ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_gpio_init(void)
{
    ESP_LOGI(TAG, "gpio init");
    return ESP_OK;
}

/* ── Read ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_gpio_read(int pin, int *level_out)
{
    if (!level_out) return ESP_ERR_INVALID_ARG;
    if (pin < 0 || pin > 48) return ESP_ERR_INVALID_ARG;

    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << pin),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    *level_out = gpio_get_level(pin);
    return ESP_OK;
}

/* ── Write ───────────────────────────────────────────────────────────────── */

esp_err_t pclika_gpio_write(int pin, int level)
{
    if (pin < 0 || pin > 48) return ESP_ERR_INVALID_ARG;
    if (is_reserved(pin)) {
        ESP_LOGW(TAG, "pin %d is reserved", pin);
        return ESP_ERR_INVALID_STATE;
    }

    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << pin),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    return gpio_set_level(pin, level ? 1 : 0);
}

/* ── Configure ───────────────────────────────────────────────────────────── */

esp_err_t pclika_gpio_configure(int pin, bool output, pclika_pull_t pull)
{
    if (pin < 0 || pin > 48) return ESP_ERR_INVALID_ARG;

    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << pin),
        .mode         = output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT,
        .pull_up_en   = (pull == PCLIKA_PULL_UP)   ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE,
        .pull_down_en = (pull == PCLIKA_PULL_DOWN)  ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    return gpio_config(&cfg);
}
