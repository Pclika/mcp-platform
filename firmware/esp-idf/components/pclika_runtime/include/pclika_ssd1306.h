/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_ssd1306.h — SSD1306 OLED 128×64 driver (I2C)
 * PCK-MMXXVI-C4A32096
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_SSD1306_ADDR_DEFAULT  0x3C   /*!< SA0=0 */
#define PCLIKA_SSD1306_ADDR_ALT      0x3D   /*!< SA0=1 */

#define PCLIKA_SSD1306_WIDTH   128
#define PCLIKA_SSD1306_HEIGHT   64
#define PCLIKA_SSD1306_PAGES     8   /*!< 64 px / 8 px per page */
#define PCLIKA_SSD1306_BUF_SIZE (PCLIKA_SSD1306_WIDTH * PCLIKA_SSD1306_PAGES)

typedef struct {
    i2c_port_t port;
    uint8_t    addr;
    uint8_t    framebuf[PCLIKA_SSD1306_BUF_SIZE]; /*!< Local shadow framebuffer */
} pclika_ssd1306_t;

/**
 * @brief  Initialise SSD1306: send init command sequence, clear display.
 */
esp_err_t pclika_ssd1306_init(i2c_port_t port, uint8_t addr, pclika_ssd1306_t *out);

/**
 * @brief  Clear framebuffer (does NOT flush to display — call flush after).
 */
void pclika_ssd1306_clear(pclika_ssd1306_t *dev);

/**
 * @brief  Flush local framebuffer to device GDDRAM.
 */
esp_err_t pclika_ssd1306_flush(pclika_ssd1306_t *dev);

/**
 * @brief  Set single pixel in framebuffer. Call flush to display.
 * @param[in] x      Column 0–127
 * @param[in] y      Row 0–63
 * @param[in] color  true = pixel on
 */
void pclika_ssd1306_pixel(pclika_ssd1306_t *dev, uint8_t x, uint8_t y, bool color);

/**
 * @brief  Draw 5×7 ASCII character at grid position (col, row).
 *         col: 0–20, row: 0–7 (page rows).
 */
void pclika_ssd1306_char(pclika_ssd1306_t *dev, uint8_t col, uint8_t row, char c);

/**
 * @brief  Draw a null-terminated string starting at (col, row).
 *         Wraps to next row on overflow.
 */
void pclika_ssd1306_string(pclika_ssd1306_t *dev, uint8_t col, uint8_t row, const char *str);

/**
 * @brief  Helper: clear, write string at top-left, flush. One-call display.
 */
esp_err_t pclika_ssd1306_print(pclika_ssd1306_t *dev, const char *text);

/**
 * @brief  Turn display on or off (contents preserved).
 */
esp_err_t pclika_ssd1306_display_on(pclika_ssd1306_t *dev, bool on);

/**
 * @brief  Set display contrast 0–255.
 */
esp_err_t pclika_ssd1306_contrast(pclika_ssd1306_t *dev, uint8_t level);

#ifdef __cplusplus
}
#endif
