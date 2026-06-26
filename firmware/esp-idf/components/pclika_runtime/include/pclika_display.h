#pragma once
/**
 * @file pclika_display.h
 * @brief Pclika Runtime — Display module
 *
 * Abstraction layer for OLED and TFT display modules.
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_DISPLAY_MAX         4
#define PCLIKA_DISPLAY_ID_MAX_LEN  32

/** Function pointer: clear the display. */
typedef esp_err_t (*pclika_display_clear_fn)(void *ctx);

/** Function pointer: write text at (col, row). */
typedef esp_err_t (*pclika_display_text_fn)(void *ctx, const char *text,
                                             int line, bool clear_first);

typedef struct {
    char                    id[PCLIKA_DISPLAY_ID_MAX_LEN];
    void                   *ctx;          /**< Driver context pointer */
    pclika_display_clear_fn clear_fn;
    pclika_display_text_fn  text_fn;
    uint8_t                 cols;         /**< Characters per line */
    uint8_t                 rows;         /**< Number of text lines */
    bool                    initialized;
} pclika_display_desc_t;

/** Initialize display subsystem. */
esp_err_t pclika_display_init(void);

/** Register a display driver. */
esp_err_t pclika_display_register(const pclika_display_desc_t *desc);

/** Write text to a display by ID. */
esp_err_t pclika_display_text(const char *display_id, const char *text,
                               int line, bool clear_first);

/** Clear a display by ID. */
esp_err_t pclika_display_clear(const char *display_id);

/** Serialize display action result to JSON. Returns bytes written. */
int pclika_display_result_to_json(bool ok, const char *error,
                                   char *buf, size_t len);

#ifdef __cplusplus
}
#endif
