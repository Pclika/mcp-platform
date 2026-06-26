/**
 * @file main.c
 * @brief hello-mcp — Pclika first example
 *
 * Demonstrates the minimum working MCP connection:
 *   - device_info returns board identity
 *   - led_control blinks the onboard LED
 *   - serial_log_read returns boot log
 *
 * Hardware: Any Pclika baseboard (ESP32-S3)
 * MCP tools exercised: device_info, led_control, serial_log_read, firmware_version
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "pclika_system.h"
#include "pclika_logging.h"
#include "pclika_gpio.h"
#include "pclika_bridge.h"

static const char *TAG = "hello_mcp";

/* Onboard LED GPIO — adjust for your board */
#define LED_GPIO    48    /* ESP32-S3 DevKitC-1 RGB LED data pin */

/* Platform seal */
const char PCLIKA_ORIGIN_SEAL[] __attribute__((used, section(".rodata.pclika"))) =
    "PCK:ORIGIN:2026-06-25:c4a32096:hello-mcp-v0.1.0";

static void led_blink_task(void *arg)
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    while (true) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    nvs_flash_init();

    pclika_logging_init();
    pclika_log_info("=== hello-mcp v0.1.0 ===");
    pclika_log_info("Platform: Pclika MCP Platform");
    pclika_log_info("Seal: PCK-MMXXVI-C4A32096");
    pclika_log_info("Connect pclika-bridge and call device_info to verify");

    pclika_system_init();
    pclika_gpio_init();

    xTaskCreate(led_blink_task, "led_blink", 2048, NULL, 3, NULL);

    pclika_log_info("LED blinking on GPIO%d", LED_GPIO);
    pclika_log_info("Bridge starting…");

    pclika_bridge_config_t cfg = PCLIKA_BRIDGE_CONFIG_DEFAULT();
    pclika_bridge_start(&cfg);

    pclika_log_info("hello-mcp ready — waiting for MCP host");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        pclika_heap_info_t h = pclika_system_get_heap();
        ESP_LOGD(TAG, "Heap free: %lu", (unsigned long)h.free_heap);
    }
}
