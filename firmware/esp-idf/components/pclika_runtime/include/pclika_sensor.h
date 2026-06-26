#pragma once
/**
 * @file pclika_sensor.h
 * @brief Pclika Runtime — Sensor module
 *
 * Unified sensor registration and read API.
 * Drivers register themselves via pclika_sensor_register().
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_SENSOR_ID_MAX_LEN  32
#define PCLIKA_SENSOR_MAX         8    /**< Max registered sensors */

/* ── Sensor reading ───────────────────────────────────────────────────────── */

typedef struct {
    float   value;          /**< Primary reading value */
    float   value2;         /**< Secondary reading (e.g., humidity when primary is temp) */
    char    unit[16];       /**< Unit string, e.g. "celsius", "%" */
    char    unit2[16];      /**< Secondary unit */
    bool    has_value2;     /**< true if value2 is valid */
    int64_t timestamp_us;   /**< ESP timestamp in microseconds */
    bool    valid;          /**< false if read failed */
    char    error[64];      /**< Error description if !valid */
} pclika_sensor_reading_t;

/* ── Driver interface ─────────────────────────────────────────────────────── */

/** Function pointer: read a sensor value. Returns ESP_OK on success. */
typedef esp_err_t (*pclika_sensor_read_fn)(int channel, pclika_sensor_reading_t *out);

typedef struct {
    char                 id[PCLIKA_SENSOR_ID_MAX_LEN]; /**< e.g. "temp_humidity", "imu" */
    pclika_sensor_read_fn read_fn;
    int                  channel_count;
    bool                 initialized;
} pclika_sensor_desc_t;

/* ── API ──────────────────────────────────────────────────────────────────── */

/** Initialize the sensor subsystem. */
esp_err_t pclika_sensor_init(void);

/**
 * @brief Register a sensor driver.
 * @param desc  Sensor descriptor. Must remain valid for the lifetime of the app.
 */
esp_err_t pclika_sensor_register(const pclika_sensor_desc_t *desc);

/**
 * @brief Read a sensor by ID and channel.
 * @param sensor_id  Sensor identifier string
 * @param channel    Channel index (0-based)
 * @param[out] out   Reading result
 */
esp_err_t pclika_sensor_read(const char *sensor_id, int channel,
                              pclika_sensor_reading_t *out);

/**
 * @brief Serialize a sensor reading to JSON.
 * @return bytes written
 */
int pclika_sensor_reading_to_json(const pclika_sensor_reading_t *r,
                                   const char *sensor_id, int channel,
                                   char *buf, size_t len);

/** List registered sensor IDs as a JSON array string. */
int pclika_sensor_list_json(char *buf, size_t len);

#ifdef __cplusplus
}
#endif
