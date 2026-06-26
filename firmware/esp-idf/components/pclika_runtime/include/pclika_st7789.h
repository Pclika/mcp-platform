/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_st7789.h — ST7789 TFT 240×240 driver (SPI)
 * PCK-MMXXVI-C4A32096
 *
 * Wiring: MOSI, SCLK, CS, DC (Data/Command), RST, (BL optional).
 * SPI Mode 3 (CPOL=1, CPHA=1), max 80 MHz.
 * Pixel format: RGB565 (16-bit/pixel, big-endian).
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_ST7789_WIDTH   240
#define PCLIKA_ST7789_HEIGHT  240

/** RGB565 colour helpers */
#define ST7789_BLACK   0x0000
#define ST7789_WHITE   0xFFFF
#define ST7789_RED     0xF800
#define ST7789_GREEN   0x07E0
#define ST7789_BLUE    0x001F
#define ST7789_YELLOW  0xFFE0
#define ST7789_CYAN    0x07FF
#define ST7789_MAGENTA 0xF81F

/** RGB888 → RGB565 macro */
#define RGB565(r, g, b) \
    ((uint16_t)(((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

typedef struct {
    spi_device_handle_t spi;
    gpio_num_t          dc_pin;   /*!< Data/Command select */
    gpio_num_t          rst_pin;  /*!< Reset (active low), -1 if unused */
    gpio_num_t          bl_pin;   /*!< Backlight PWM/GPIO, -1 if unused */
    uint16_t            width;
    uint16_t            height;
    uint8_t             col_offset;  /*!< Column start offset (some modules use 0, some 80) */
    uint8_t             row_offset;
} pclika_st7789_t;

typedef struct {
    spi_host_device_t   host;
    gpio_num_t          mosi;
    gpio_num_t          sclk;
    gpio_num_t          cs;
    gpio_num_t          dc;
    gpio_num_t          rst;     /*!< -1 if not connected */
    gpio_num_t          bl;      /*!< -1 if not connected */
    int                 freq_hz; /*!< SPI clock frequency. Default: 40000000 */
    uint8_t             col_offset;
    uint8_t             row_offset;
} pclika_st7789_config_t;

/**
 * @brief  Initialise ST7789: configure SPI, send init sequence, clear screen.
 */
esp_err_t pclika_st7789_init(const pclika_st7789_config_t *cfg, pclika_st7789_t *out);

/**
 * @brief  Fill the entire screen with a colour.
 */
esp_err_t pclika_st7789_fill(pclika_st7789_t *dev, uint16_t color);

/**
 * @brief  Draw a filled rectangle.
 */
esp_err_t pclika_st7789_fill_rect(pclika_st7789_t *dev,
                                   uint16_t x, uint16_t y,
                                   uint16_t w, uint16_t h,
                                   uint16_t color);

/**
 * @brief  Blit a raw RGB565 pixel buffer into a rectangular window.
 * @param[in] pixels  Buffer of w×h uint16_t pixels in big-endian RGB565.
 */
esp_err_t pclika_st7789_blit(pclika_st7789_t *dev,
                              uint16_t x, uint16_t y,
                              uint16_t w, uint16_t h,
                              const uint16_t *pixels);

/**
 * @brief  Draw a single pixel.
 */
esp_err_t pclika_st7789_pixel(pclika_st7789_t *dev,
                               uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief  Set backlight duty (0 = off, 255 = full). GPIO toggle if no PWM.
 */
esp_err_t pclika_st7789_backlight(pclika_st7789_t *dev, uint8_t brightness);

/**
 * @brief  Draw 6×8 ASCII character at pixel position (x, y) in RGB565.
 */
void pclika_st7789_char(pclika_st7789_t *dev, uint16_t x, uint16_t y,
                         char c, uint16_t fg, uint16_t bg);

/**
 * @brief  Draw null-terminated string.
 */
void pclika_st7789_string(pclika_st7789_t *dev, uint16_t x, uint16_t y,
                           const char *str, uint16_t fg, uint16_t bg);

#ifdef __cplusplus
}
#endif
