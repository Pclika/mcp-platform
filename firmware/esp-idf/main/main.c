/**
 * @file main.c
 * @brief Pclika MCP Platform — ESP32-S3 main entry point
 *
 * Boot sequence:
 *   1. NVS init
 *   2. Runtime modules init (system, gpio, sensor, display, servo, wifi, logging)
 *   3. Bridge start (UART/USB-CDC MCP listener)
 *
 * After bridge start, the device responds to NDJSON commands from pclika-bridge
 * running on the host, which forwards them from Claude/Codex/Cursor.
 *
 * Platform seal: PCK-MMXXVI-C4A32096
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "pclika_system.h"
#include "pclika_logging.h"
#include "pclika_sensor.h"
#include "pclika_display.h"
#include "pclika_servo.h"
#include "pclika_gpio.h"
#include "pclika_wifi.h"
#include "pclika_bridge.h"

static const char *TAG = "pclika_main";

/* Platform integrity marker — embedded in .rodata, extractable via `strings firmware.bin` */
const char PCLIKA_ORIGIN_SEAL[] __attribute__((used, section(".rodata.pclika"))) =
    "PCK:ORIGIN:2026-06-25:c4a32096"
    ":Pclika is not a board."
    ":It is the layer between hardware and intelligence.";

void app_main(void)
{
    /* ── NVS ─────────────────────────────────────────────────────────────── */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    /* ── Logging ─────────────────────────────────────────────────────────── */
    pclika_logging_init();
    pclika_log_info("Pclika MCP Platform v%s booting…", PCLIKA_FW_VERSION);
    pclika_log_info("Seal: PCK-MMXXVI-C4A32096");

    /* ── System ──────────────────────────────────────────────────────────── */
    pclika_system_init();

    char board_id[32];
    pclika_system_get_board_id(board_id, sizeof(board_id));
    pclika_log_info("Board ID: %s", board_id);

    /* ── GPIO ────────────────────────────────────────────────────────────── */
    pclika_gpio_init();

    /* ── Sensor subsystem ────────────────────────────────────────────────── */
    pclika_sensor_init();
    /*
     * Register sensor drivers here, e.g.:
     *   extern esp_err_t dht22_sensor_register(int gpio_pin);
     *   dht22_sensor_register(4);
     */

    /* ── Display subsystem ───────────────────────────────────────────────── */
    pclika_display_init();
    /*
     * Register display drivers here, e.g.:
     *   extern esp_err_t ssd1306_display_register(int sda, int scl);
     *   ssd1306_display_register(8, 9);
     */

    /* ── Servo subsystem ─────────────────────────────────────────────────── */
    pclika_servo_init();

    /* ── Wi-Fi ───────────────────────────────────────────────────────────── */
    pclika_wifi_init();

    /* ── Log heap at boot ────────────────────────────────────────────────── */
    pclika_heap_info_t heap = pclika_system_get_heap();
    pclika_log_info("Heap: %lu free / %lu total",
                    (unsigned long)heap.free_heap,
                    (unsigned long)heap.total_heap);

    /* ── Start MCP Bridge ────────────────────────────────────────────────── */
    pclika_bridge_config_t bridge_cfg = PCLIKA_BRIDGE_CONFIG_DEFAULT();
    err = pclika_bridge_start(&bridge_cfg);
    if (err != ESP_OK) {
        pclika_log_error("Bridge start failed: %s", esp_err_to_name(err));
        /* Device still boots and logs; bridge can be retried */
    } else {
        pclika_log_info("MCP Bridge ready — waiting for host commands");
    }

    /* Main task parks here; bridge runs in its own task */
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));

        heap = pclika_system_get_heap();
        ESP_LOGD(TAG, "Heap: %lu free", (unsigned long)heap.free_heap);
    }
}
