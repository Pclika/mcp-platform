/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * pclika_st7789.c — ST7789 TFT 240×240 driver (SPI)
 * PCK-MMXXVI-C4A32096
 */
#include "pclika_st7789.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

#define TAG "pclika_st7789"

/* ST7789 command bytes */
#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT  0x11
#define ST7789_NORON   0x13
#define ST7789_INVON   0x21
#define ST7789_DISPON  0x29
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_MADCTL  0x36
#define ST7789_COLMOD  0x3A
#define ST7789_PORCTRL 0xB2
#define ST7789_GCTRL   0xB7
#define ST7789_VCOMS   0xBB
#define ST7789_LCMCTRL 0xC0
#define ST7789_VDVVRHEN 0xC2
#define ST7789_VRHS    0xC3
#define ST7789_VDVSET  0xC4
#define ST7789_FRCTR2  0xC6
#define ST7789_PWCTRL1 0xD0
#define ST7789_PVGAMCTRL 0xE0
#define ST7789_NVGAMCTRL 0xE1

/* SPI DMA line-buffer size (bytes); send fills in 240-px lines */
#define LINE_BUF_PIXELS 240
#define LINE_BUF_BYTES  (LINE_BUF_PIXELS * 2)

/* -------- Low-level SPI helpers -------- */

static void dc_set(pclika_st7789_t *dev, bool data)
{
    gpio_set_level(dev->dc_pin, data ? 1 : 0);
}

static esp_err_t spi_write(pclika_st7789_t *dev, const uint8_t *data, size_t len)
{
    if (len == 0) return ESP_OK;
    spi_transaction_t t = {
        .length    = len * 8,
        .tx_buffer = data,
    };
    return spi_device_transmit(dev->spi, &t);
}

static esp_err_t send_cmd(pclika_st7789_t *dev, uint8_t cmd)
{
    dc_set(dev, false);
    return spi_write(dev, &cmd, 1);
}

static esp_err_t send_data(pclika_st7789_t *dev, const uint8_t *data, size_t len)
{
    dc_set(dev, true);
    return spi_write(dev, data, len);
}

static esp_err_t send_data8(pclika_st7789_t *dev, uint8_t d)
{
    return send_data(dev, &d, 1);
}

/* -------- Address window -------- */

static esp_err_t set_window(pclika_st7789_t *dev,
                             uint16_t x1, uint16_t y1,
                             uint16_t x2, uint16_t y2)
{
    x1 += dev->col_offset; x2 += dev->col_offset;
    y1 += dev->row_offset; y2 += dev->row_offset;

    uint8_t col[4] = {x1 >> 8, x1 & 0xFF, x2 >> 8, x2 & 0xFF};
    uint8_t row[4] = {y1 >> 8, y1 & 0xFF, y2 >> 8, y2 & 0xFF};

    esp_err_t ret;
    ret = send_cmd(dev, ST7789_CASET); if (ret) return ret;
    ret = send_data(dev, col, 4);      if (ret) return ret;
    ret = send_cmd(dev, ST7789_RASET); if (ret) return ret;
    ret = send_data(dev, row, 4);      if (ret) return ret;
    return send_cmd(dev, ST7789_RAMWR);
}

/* -------- Init sequence -------- */

static esp_err_t run_init(pclika_st7789_t *dev)
{
    /* Hard reset */
    if (dev->rst_pin >= 0) {
        gpio_set_level(dev->rst_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(dev->rst_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(120));
    }

#define CMD(c)    do { if ((ret = send_cmd(dev, c))) return ret; } while(0)
#define DAT(d)    do { if ((ret = send_data8(dev, d))) return ret; } while(0)

    esp_err_t ret;
    CMD(ST7789_SWRESET); vTaskDelay(pdMS_TO_TICKS(150));
    CMD(ST7789_SLPOUT);  vTaskDelay(pdMS_TO_TICKS(10));
    CMD(ST7789_COLMOD);  DAT(0x55);  /* 16-bit RGB565 */
    CMD(ST7789_MADCTL);  DAT(0x00);  /* normal orientation */
    CMD(ST7789_PORCTRL); DAT(0x0C); DAT(0x0C); DAT(0x00); DAT(0x33); DAT(0x33);
    CMD(ST7789_GCTRL);   DAT(0x35);
    CMD(ST7789_VCOMS);   DAT(0x19);
    CMD(ST7789_LCMCTRL); DAT(0x2C);
    CMD(ST7789_VDVVRHEN);DAT(0x01);
    CMD(ST7789_VRHS);    DAT(0x12);
    CMD(ST7789_VDVSET);  DAT(0x20);
    CMD(ST7789_FRCTR2);  DAT(0x0F);
    CMD(ST7789_PWCTRL1); DAT(0xA4); DAT(0xA1);
    CMD(ST7789_PVGAMCTRL);
    DAT(0xD0);DAT(0x04);DAT(0x0D);DAT(0x11);DAT(0x13);DAT(0x2B);
    DAT(0x3F);DAT(0x54);DAT(0x4C);DAT(0x18);DAT(0x0D);DAT(0x0B);
    DAT(0x1F);DAT(0x23);
    CMD(ST7789_NVGAMCTRL);
    DAT(0xD0);DAT(0x04);DAT(0x0C);DAT(0x11);DAT(0x13);DAT(0x2C);
    DAT(0x3F);DAT(0x44);DAT(0x51);DAT(0x2F);DAT(0x1F);DAT(0x1F);
    DAT(0x20);DAT(0x23);
    CMD(ST7789_INVON);   /* inversion needed for correct colour on most 240×240 panels */
    CMD(ST7789_NORON);
    CMD(ST7789_DISPON);  vTaskDelay(pdMS_TO_TICKS(5));

#undef CMD
#undef DAT
    return ESP_OK;
}

/* -------- Public API -------- */

esp_err_t pclika_st7789_init(const pclika_st7789_config_t *cfg, pclika_st7789_t *out)
{
    out->dc_pin     = cfg->dc;
    out->rst_pin    = cfg->rst;
    out->bl_pin     = cfg->bl;
    out->width      = PCLIKA_ST7789_WIDTH;
    out->height     = PCLIKA_ST7789_HEIGHT;
    out->col_offset = cfg->col_offset;
    out->row_offset = cfg->row_offset;

    /* Configure DC, RST, BL pins */
    gpio_reset_pin(cfg->dc);
    gpio_set_direction(cfg->dc, GPIO_MODE_OUTPUT);
    if (cfg->rst >= 0) { gpio_reset_pin(cfg->rst); gpio_set_direction(cfg->rst, GPIO_MODE_OUTPUT); gpio_set_level(cfg->rst, 1); }
    if (cfg->bl  >= 0) { gpio_reset_pin(cfg->bl);  gpio_set_direction(cfg->bl,  GPIO_MODE_OUTPUT); gpio_set_level(cfg->bl,  1); }

    /* SPI bus add device */
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = cfg->freq_hz ? cfg->freq_hz : 40000000,
        .mode           = 3,   /* CPOL=1, CPHA=1 */
        .spics_io_num   = cfg->cs,
        .queue_size     = 7,
    };
    esp_err_t ret = spi_bus_add_device(cfg->host, &devcfg, &out->spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = run_init(out);
    if (ret != ESP_OK) return ret;

    ret = pclika_st7789_fill(out, ST7789_BLACK);
    ESP_LOGI(TAG, "init OK %dx%d col_off=%d row_off=%d",
             out->width, out->height, out->col_offset, out->row_offset);
    return ret;
}

esp_err_t pclika_st7789_fill(pclika_st7789_t *dev, uint16_t color)
{
    return pclika_st7789_fill_rect(dev, 0, 0, dev->width, dev->height, color);
}

esp_err_t pclika_st7789_fill_rect(pclika_st7789_t *dev,
                                   uint16_t x, uint16_t y,
                                   uint16_t w, uint16_t h,
                                   uint16_t color)
{
    if (x + w > dev->width || y + h > dev->height) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = set_window(dev, x, y, x + w - 1, y + h - 1);
    if (ret != ESP_OK) return ret;

    /* Build line buffer filled with colour (big-endian) */
    static uint8_t linebuf[LINE_BUF_BYTES];
    uint8_t hi = color >> 8, lo = color & 0xFF;
    for (int i = 0; i < LINE_BUF_PIXELS; i++) { linebuf[i*2] = hi; linebuf[i*2+1] = lo; }

    dc_set(dev, true);
    uint32_t total_pixels = (uint32_t)w * h;
    while (total_pixels > 0) {
        uint32_t chunk = total_pixels > LINE_BUF_PIXELS ? LINE_BUF_PIXELS : total_pixels;
        ret = spi_write(dev, linebuf, chunk * 2);
        if (ret != ESP_OK) return ret;
        total_pixels -= chunk;
    }
    return ESP_OK;
}

esp_err_t pclika_st7789_blit(pclika_st7789_t *dev,
                              uint16_t x, uint16_t y,
                              uint16_t w, uint16_t h,
                              const uint16_t *pixels)
{
    esp_err_t ret = set_window(dev, x, y, x + w - 1, y + h - 1);
    if (ret != ESP_OK) return ret;
    dc_set(dev, true);
    return spi_write(dev, (const uint8_t *)pixels, (size_t)w * h * 2);
}

esp_err_t pclika_st7789_pixel(pclika_st7789_t *dev,
                               uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t buf[2] = {color >> 8, color & 0xFF};
    esp_err_t ret = set_window(dev, x, y, x, y);
    if (ret != ESP_OK) return ret;
    dc_set(dev, true);
    return spi_write(dev, buf, 2);
}

esp_err_t pclika_st7789_backlight(pclika_st7789_t *dev, uint8_t brightness)
{
    if (dev->bl_pin < 0) return ESP_ERR_NOT_SUPPORTED;
    gpio_set_level(dev->bl_pin, brightness > 0 ? 1 : 0);
    return ESP_OK;
}

/* 5×7 font reuse from SSD1306 driver — duplicated here for independence */
static const uint8_t FONT5X7_ST[][5] = {
    {0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00},{0x00,0x07,0x00,0x07,0x00},
    {0x14,0x7F,0x14,0x7F,0x14},{0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,0x08,0x64,0x62},
    {0x36,0x49,0x55,0x22,0x50},{0x00,0x05,0x03,0x00,0x00},{0x00,0x1C,0x22,0x41,0x00},
    {0x00,0x41,0x22,0x1C,0x00},{0x14,0x08,0x3E,0x08,0x14},{0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x50,0x30,0x00,0x00},{0x08,0x08,0x08,0x08,0x08},{0x00,0x60,0x60,0x00,0x00},
    {0x20,0x10,0x08,0x04,0x02},{0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},{0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E},{0x00,0x36,0x36,0x00,0x00},
    {0x00,0x56,0x36,0x00,0x00},{0x08,0x14,0x22,0x41,0x00},{0x14,0x14,0x14,0x14,0x14},
    {0x00,0x41,0x22,0x14,0x08},{0x02,0x01,0x51,0x09,0x06},{0x32,0x49,0x79,0x41,0x3E},
    {0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A},{0x7F,0x08,0x08,0x08,0x7F},{0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},{0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7F,0x01,0x01},{0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},{0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},{0x20,0x54,0x54,0x54,0x78},
    {0x7F,0x48,0x44,0x44,0x38},{0x38,0x44,0x44,0x44,0x20},{0x38,0x44,0x44,0x48,0x7F},
    {0x38,0x54,0x54,0x54,0x18},{0x08,0x7E,0x09,0x01,0x02},{0x0C,0x52,0x52,0x52,0x3E},
    {0x7F,0x08,0x04,0x04,0x78},{0x00,0x44,0x7D,0x40,0x00},{0x20,0x40,0x44,0x3D,0x00},
    {0x7F,0x10,0x28,0x44,0x00},{0x00,0x41,0x7F,0x40,0x00},{0x7C,0x04,0x18,0x04,0x78},
    {0x7C,0x08,0x04,0x04,0x78},{0x38,0x44,0x44,0x44,0x38},{0x7C,0x14,0x14,0x14,0x08},
    {0x08,0x14,0x14,0x18,0x7C},{0x7C,0x08,0x04,0x04,0x08},{0x48,0x54,0x54,0x54,0x20},
    {0x04,0x3F,0x44,0x40,0x20},{0x3C,0x40,0x40,0x20,0x7C},{0x1C,0x20,0x40,0x20,0x1C},
    {0x3C,0x40,0x30,0x40,0x3C},{0x44,0x28,0x10,0x28,0x44},{0x0C,0x50,0x50,0x50,0x3C},
    {0x44,0x64,0x54,0x4C,0x44},
};

void pclika_st7789_char(pclika_st7789_t *dev, uint16_t x, uint16_t y,
                         char c, uint16_t fg, uint16_t bg)
{
    if (c < 0x20 || c > 0x7A) c = '?';
    const uint8_t *glyph = FONT5X7_ST[c - 0x20];
    /* Each glyph is 5 columns of 7 rows; draw as 6×8 with padding */
    uint16_t buf[6 * 8];
    for (uint8_t col = 0; col < 6; col++) {
        uint8_t bits = (col < 5) ? glyph[col] : 0;
        for (uint8_t row = 0; row < 8; row++) {
            buf[row * 6 + col] = (bits & (1 << row)) ? fg : bg;
        }
    }
    pclika_st7789_blit(dev, x, y, 6, 8, buf);
}

void pclika_st7789_string(pclika_st7789_t *dev, uint16_t x, uint16_t y,
                           const char *str, uint16_t fg, uint16_t bg)
{
    uint16_t cx = x;
    while (*str) {
        if (*str == '\n') { cx = x; y += 9; str++; continue; }
        if (cx + 6 > dev->width) { cx = x; y += 9; }
        if (y + 8 > dev->height) break;
        pclika_st7789_char(dev, cx, y, *str, fg, bg);
        cx += 6;
        str++;
    }
}
