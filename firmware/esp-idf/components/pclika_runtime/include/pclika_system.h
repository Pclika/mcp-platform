#pragma once
/**
 * @file pclika_system.h
 * @brief Pclika Runtime — System module
 *
 * Board identity, firmware version, capability flags, and heap diagnostics.
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Platform seal ─────────────────────────────────────────────────────────── */

/** Embedded in all official Pclika firmware. Extractable via strings. */
extern const char PCLIKA_ORIGIN_SEAL[];

/* ── Version info ──────────────────────────────────────────────────────────── */

#define PCLIKA_FW_VERSION       "0.1.0"
#define PCLIKA_FW_BUILD_DATE    __DATE__ " " __TIME__
#define PCLIKA_PLATFORM         "esp32s3"
#define PCLIKA_BOARD_VARIANT    "pclika-mcp-basic"

/* ── Capability flags ─────────────────────────────────────────────────────── */

typedef struct {
    bool has_sensor;      /**< At least one sensor module registered */
    bool has_display;     /**< At least one display registered */
    bool has_servo;       /**< Servo/PWM output configured */
    bool has_gpio;        /**< GPIO control available */
    bool has_wifi;        /**< Wi-Fi subsystem available */
    bool has_ble;         /**< BLE subsystem available */
    bool has_camera;      /**< Camera module registered */
    bool has_led_rgb;     /**< Addressable RGB LED available */
} pclika_caps_t;

/* ── Heap info ────────────────────────────────────────────────────────────── */

typedef struct {
    uint32_t free_heap;       /**< Current free heap bytes */
    uint32_t min_free_heap;   /**< Minimum free heap ever (watermark) */
    uint32_t total_heap;      /**< Total heap at boot */
} pclika_heap_info_t;

/* ── API ──────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the system module. Call once at boot.
 */
esp_err_t pclika_system_init(void);

/**
 * @brief Get the board unique identifier (derived from MAC address).
 * @param[out] buf  Buffer to fill (at least 18 bytes, format: XX:XX:XX:XX:XX:XX)
 */
void pclika_system_get_board_id(char *buf, size_t len);

/**
 * @brief Get current capability flags.
 */
pclika_caps_t pclika_system_get_caps(void);

/**
 * @brief Update a single capability flag (called by subsystem init functions).
 */
void pclika_system_set_cap(const char *cap_name, bool value);

/**
 * @brief Get heap diagnostics.
 */
pclika_heap_info_t pclika_system_get_heap(void);

/**
 * @brief Serialize system info to a JSON string.
 * @param[out] buf   Output buffer
 * @param[in]  len   Buffer length
 * @return Number of bytes written (excluding null terminator)
 */
int pclika_system_to_json(char *buf, size_t len);

#ifdef __cplusplus
}
#endif
