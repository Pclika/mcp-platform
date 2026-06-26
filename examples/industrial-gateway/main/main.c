/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * industrial-gateway — MCP Example
 * PCK-MMXXVI-C4A32096
 *
 * Demonstrates industrial I/O via MCP:
 *   - Modbus RTU over RS-485 (UART2, using ESP-Modbus or raw framing)
 *   - Relay bank control (8-channel via GPIO or I2C expander)
 *   - Multi-channel ADC read (built-in ADC1 channels 0–3)
 *   - Digital I/O read (GPIO bank)
 *
 * MCP tools exposed: modbus_read, modbus_write, relay_set,
 *                    digital_io_read, adc_read_multi
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "cJSON.h"

#include "pclika_bridge.h"
#include "pclika_runtime.h"

#define TAG "industrial-gateway"

/* -------- Pin / peripheral config -------- */
#define RS485_UART_NUM   UART_NUM_2
#define RS485_TX_PIN     17
#define RS485_RX_PIN     16
#define RS485_DE_PIN     5      /* Driver enable — high = transmit */
#define RS485_BAUD       9600

/* Relay GPIOs (active-HIGH) */
static const gpio_num_t RELAY_PINS[8] = {
    GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_21, GPIO_NUM_22,
    GPIO_NUM_23, GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27,
};
#define RELAY_COUNT 8

/* Digital input GPIOs */
static const gpio_num_t DIN_PINS[8] = {
    GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_35,
    GPIO_NUM_36, GPIO_NUM_39, GPIO_NUM_NC, GPIO_NUM_NC,
};
#define DIN_COUNT 6   /* 34-39 are input-only on ESP32 */

/* ADC channels */
static const adc1_channel_t ADC_CH[4] = {
    ADC1_CHANNEL_0, ADC1_CHANNEL_3, ADC1_CHANNEL_6, ADC1_CHANNEL_7,
};
#define ADC_COUNT 4
#define ADC_ATTEN ADC_ATTEN_DB_11   /* 0–3.9 V range */
static esp_adc_cal_characteristics_t s_adc_chars;

/* -------- Modbus RTU (minimal raw implementation) -------- */

static uint16_t modbus_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
        }
    }
    return crc;
}

typedef struct {
    uint8_t  slave_id;
    uint8_t  function;
    uint16_t address;
    uint16_t count;
    uint16_t values[125];
    uint8_t  error_code;
    bool     ok;
} modbus_result_t;

static esp_err_t modbus_transaction(uint8_t slave, uint8_t func,
                                     uint16_t addr, uint16_t count,
                                     const uint16_t *write_vals,
                                     modbus_result_t *out,
                                     uint32_t timeout_ms)
{
    uint8_t req[256];
    size_t  req_len = 0;

    req[req_len++] = slave;
    req[req_len++] = func;

    switch (func) {
        case 0x01: case 0x02: case 0x03: case 0x04:
            /* Read coils/inputs/holding/input registers */
            req[req_len++] = addr >> 8;   req[req_len++] = addr & 0xFF;
            req[req_len++] = count >> 8;  req[req_len++] = count & 0xFF;
            break;
        case 0x05:
            /* Write single coil */
            req[req_len++] = addr >> 8;   req[req_len++] = addr & 0xFF;
            req[req_len++] = write_vals[0] ? 0xFF : 0x00;
            req[req_len++] = 0x00;
            break;
        case 0x06:
            /* Write single register */
            req[req_len++] = addr >> 8;   req[req_len++] = addr & 0xFF;
            req[req_len++] = write_vals[0] >> 8;
            req[req_len++] = write_vals[0] & 0xFF;
            break;
        case 0x10:
            /* Write multiple registers */
            req[req_len++] = addr >> 8;   req[req_len++] = addr & 0xFF;
            req[req_len++] = count >> 8;  req[req_len++] = count & 0xFF;
            req[req_len++] = (uint8_t)(count * 2);
            for (uint16_t i = 0; i < count; i++) {
                req[req_len++] = write_vals[i] >> 8;
                req[req_len++] = write_vals[i] & 0xFF;
            }
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }

    uint16_t crc = modbus_crc16(req, req_len);
    req[req_len++] = crc & 0xFF;
    req[req_len++] = crc >> 8;

    /* DE high = transmit */
    gpio_set_level(RS485_DE_PIN, 1);
    uart_write_bytes(RS485_UART_NUM, (char *)req, req_len);
    uart_wait_tx_done(RS485_UART_NUM, pdMS_TO_TICKS(20));
    gpio_set_level(RS485_DE_PIN, 0);  /* DE low = receive */

    /* Read response */
    uint8_t  resp[256];
    int      resp_len = uart_read_bytes(RS485_UART_NUM, resp,
                                         sizeof(resp),
                                         pdMS_TO_TICKS(timeout_ms));
    if (resp_len < 4) {
        out->ok = false;
        return ESP_ERR_TIMEOUT;
    }

    /* Verify CRC */
    uint16_t resp_crc = modbus_crc16(resp, resp_len - 2);
    uint16_t rx_crc   = (uint16_t)resp[resp_len-1] << 8 | resp[resp_len-2];
    if (resp_crc != rx_crc) {
        out->ok = false;
        return ESP_ERR_INVALID_CRC;
    }

    /* Check exception response */
    if (resp[1] & 0x80) {
        out->ok         = false;
        out->error_code = resp[2];
        return ESP_FAIL;
    }

    out->ok       = true;
    out->slave_id = resp[0];
    out->function = resp[1];

    /* Parse read response data */
    if (func <= 0x04) {
        uint8_t byte_count = resp[2];
        out->count = byte_count / 2;
        for (uint8_t i = 0; i < out->count && i < 125; i++) {
            out->values[i] = ((uint16_t)resp[3 + i*2] << 8) | resp[4 + i*2];
        }
    } else {
        out->address = ((uint16_t)resp[2] << 8) | resp[3];
        out->count   = ((uint16_t)resp[4] << 8) | resp[5];
    }
    return ESP_OK;
}

/* -------- MCP tool: modbus_read -------- */

static void tool_modbus_read(const cJSON *params, cJSON *result)
{
    uint8_t  slave   = (uint8_t)cJSON_GetObjectItem(params, "slave_id")->valueint;
    const char *func = cJSON_GetObjectItem(params, "function")->valuestring;
    uint16_t addr    = (uint16_t)cJSON_GetObjectItem(params, "address")->valueint;
    uint16_t count   = (uint16_t)cJSON_GetObjectItem(params, "count")->valueint;
    uint32_t timeout = 500;
    cJSON *j = cJSON_GetObjectItem(params, "timeout_ms");
    if (cJSON_IsNumber(j)) timeout = (uint32_t)j->valueint;

    uint8_t fc = 0x03;  /* default: holding registers */
    if (strcmp(func, "input_registers")  == 0) fc = 0x04;
    else if (strcmp(func, "coils")       == 0) fc = 0x01;
    else if (strcmp(func, "discrete_inputs") == 0) fc = 0x02;

    modbus_result_t mb = {0};
    esp_err_t ret = modbus_transaction(slave, fc, addr, count, NULL, &mb, timeout);
    if (ret != ESP_OK || !mb.ok) {
        cJSON_AddBoolToObject(result, "ok", false);
        cJSON_AddStringToObject(result, "error", esp_err_to_name(ret));
        return;
    }

    cJSON_AddBoolToObject(result,   "ok",       true);
    cJSON_AddNumberToObject(result, "slave_id", slave);
    cJSON_AddNumberToObject(result, "address",  addr);
    cJSON_AddNumberToObject(result, "count",    mb.count);
    cJSON *vals = cJSON_AddArrayToObject(result, "values");
    for (uint16_t i = 0; i < mb.count; i++) {
        cJSON_AddItemToArray(vals, cJSON_CreateNumber(mb.values[i]));
    }
}

/* -------- MCP tool: modbus_write -------- */

static void tool_modbus_write(const cJSON *params, cJSON *result)
{
    uint8_t  slave   = (uint8_t)cJSON_GetObjectItem(params, "slave_id")->valueint;
    const char *func = cJSON_GetObjectItem(params, "function")->valuestring;
    uint16_t addr    = (uint16_t)cJSON_GetObjectItem(params, "address")->valueint;
    cJSON *vals_arr  = cJSON_GetObjectItem(params, "values");
    uint16_t count   = (uint16_t)cJSON_GetArraySize(vals_arr);

    uint16_t vals[125] = {0};
    for (uint16_t i = 0; i < count && i < 125; i++) {
        vals[i] = (uint16_t)cJSON_GetArrayItem(vals_arr, i)->valueint;
    }

    uint8_t fc = (strcmp(func, "coils") == 0)
                 ? (count == 1 ? 0x05 : 0x0F)
                 : (count == 1 ? 0x06 : 0x10);

    modbus_result_t mb = {0};
    esp_err_t ret = modbus_transaction(slave, fc, addr, count, vals, &mb, 500);
    cJSON_AddBoolToObject(result, "ok", ret == ESP_OK && mb.ok);
    if (ret == ESP_OK && mb.ok) {
        cJSON_AddNumberToObject(result, "slave_id",      slave);
        cJSON_AddNumberToObject(result, "address",       addr);
        cJSON_AddNumberToObject(result, "values_written",count);
    } else {
        cJSON_AddStringToObject(result, "error", esp_err_to_name(ret));
    }
}

/* -------- MCP tool: relay_set -------- */

static bool s_relay_state[RELAY_COUNT] = {false};

static void tool_relay_set(const cJSON *params, cJSON *result)
{
    int channel = cJSON_GetObjectItem(params, "channel")->valueint;
    const char *state_str = cJSON_GetObjectItem(params, "state")->valuestring;
    cJSON *j = cJSON_GetObjectItem(params, "pulse_ms");
    int pulse_ms = cJSON_IsNumber(j) ? j->valueint : 0;

    int ch_start = (channel == -1) ? 0       : channel;
    int ch_end   = (channel == -1) ? RELAY_COUNT : channel + 1;

    for (int ch = ch_start; ch < ch_end; ch++) {
        bool new_state;
        if (strcmp(state_str, "on")     == 0) new_state = true;
        else if (strcmp(state_str, "off") == 0) new_state = false;
        else /* toggle */                      new_state = !s_relay_state[ch];

        gpio_set_level(RELAY_PINS[ch], new_state ? 1 : 0);
        s_relay_state[ch] = new_state;
    }

    /* Report first affected channel */
    int rep_ch = (channel == -1) ? 0 : channel;
    cJSON_AddBoolToObject(result, "ok",      true);
    cJSON_AddNumberToObject(result, "channel", rep_ch);
    cJSON_AddStringToObject(result, "state",   s_relay_state[rep_ch] ? "on" : "off");

    cJSON *all = cJSON_AddArrayToObject(result, "all_states");
    for (int ch = 0; ch < RELAY_COUNT; ch++) {
        cJSON_AddItemToArray(all, cJSON_CreateString(s_relay_state[ch] ? "on" : "off"));
    }

    /* Pulse: restore after delay (fire-and-forget task) */
    if (pulse_ms > 0 && channel >= 0) {
        /* Simple blocking wait — for production use a timer task */
        vTaskDelay(pdMS_TO_TICKS(pulse_ms));
        bool restore = !s_relay_state[channel];
        gpio_set_level(RELAY_PINS[channel], restore ? 1 : 0);
        s_relay_state[channel] = restore;
    }
}

/* -------- MCP tool: digital_io_read -------- */

static void tool_digital_io_read(const cJSON *params, cJSON *result)
{
    (void)params;
    cJSON *levels = cJSON_AddArrayToObject(result, "levels");
    uint8_t mask = 0;
    for (int i = 0; i < DIN_COUNT; i++) {
        int lvl = gpio_get_level(DIN_PINS[i]);
        cJSON_AddItemToArray(levels, cJSON_CreateNumber(lvl));
        if (lvl) mask |= (1 << i);
    }
    cJSON_AddNumberToObject(result, "bank", 0);
    cJSON_AddNumberToObject(result, "mask", mask);
}

/* -------- MCP tool: adc_read_multi -------- */

static void tool_adc_read_multi(const cJSON *params, cJSON *result)
{
    cJSON *ch_arr = cJSON_GetObjectItem(params, "channels");
    float vref = 3.3f;
    cJSON *j = cJSON_GetObjectItem(params, "vref");
    if (cJSON_IsNumber(j)) vref = (float)j->valuedouble;

    cJSON *readings = cJSON_AddArrayToObject(result, "readings");
    int n = cJSON_GetArraySize(ch_arr);
    for (int i = 0; i < n; i++) {
        int ch_idx = cJSON_GetArrayItem(ch_arr, i)->valueint;
        if (ch_idx < 0 || ch_idx >= ADC_COUNT) continue;

        uint32_t voltage_mv = 0;
        /* Average 4 samples */
        for (int s = 0; s < 4; s++) {
            voltage_mv += esp_adc_cal_raw_to_voltage(
                adc1_get_raw(ADC_CH[ch_idx]), &s_adc_chars);
        }
        voltage_mv /= 4;

        uint32_t raw = adc1_get_raw(ADC_CH[ch_idx]);

        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "channel", ch_idx);
        cJSON_AddNumberToObject(entry, "raw",     raw);
        cJSON_AddNumberToObject(entry, "voltage", voltage_mv / 1000.0);
        cJSON_AddItemToArray(readings, entry);
    }
    cJSON_AddStringToObject(result, "adc_source",   "internal");
    cJSON_AddNumberToObject(result, "vref",          vref);
    cJSON_AddNumberToObject(result, "timestamp_ms",
                            (double)(esp_timer_get_time() / 1000));
}

/* -------- Hardware init -------- */

static void relay_gpio_init(void)
{
    for (int i = 0; i < RELAY_COUNT; i++) {
        gpio_reset_pin(RELAY_PINS[i]);
        gpio_set_direction(RELAY_PINS[i], GPIO_MODE_OUTPUT);
        gpio_set_level(RELAY_PINS[i], 0);
    }
}

static void din_gpio_init(void)
{
    for (int i = 0; i < DIN_COUNT; i++) {
        gpio_reset_pin(DIN_PINS[i]);
        gpio_set_direction(DIN_PINS[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(DIN_PINS[i], GPIO_PULLDOWN_ONLY);
    }
}

static void rs485_init(void)
{
    uart_config_t cfg = {
        .baud_rate  = RS485_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(RS485_UART_NUM, 256, 256, 0, NULL, 0);
    uart_param_config(RS485_UART_NUM, &cfg);
    uart_set_pin(RS485_UART_NUM, RS485_TX_PIN, RS485_RX_PIN,
                 RS485_DE_PIN, UART_PIN_NO_CHANGE);
    uart_set_mode(RS485_UART_NUM, UART_MODE_RS485_HALF_DUPLEX);
    ESP_LOGI(TAG, "RS-485 ready @ %d baud", RS485_BAUD);
}

static void adc_init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    for (int i = 0; i < ADC_COUNT; i++) {
        adc1_config_channel_atten(ADC_CH[i], ADC_ATTEN);
    }
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT_12,
                              1100, &s_adc_chars);
    ESP_LOGI(TAG, "ADC %d channels ready", ADC_COUNT);
}

/* -------- app_main -------- */

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    pclika_runtime_config_t rt_cfg = PCLIKA_RUNTIME_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(pclika_runtime_init(&rt_cfg));

    relay_gpio_init();
    din_gpio_init();
    rs485_init();
    adc_init();

    pclika_bridge_register_tool("modbus_read",      tool_modbus_read);
    pclika_bridge_register_tool("modbus_write",     tool_modbus_write);
    pclika_bridge_register_tool("relay_set",        tool_relay_set);
    pclika_bridge_register_tool("digital_io_read",  tool_digital_io_read);
    pclika_bridge_register_tool("adc_read_multi",   tool_adc_read_multi);

    ESP_LOGI(TAG, "industrial-gateway ready — 5 MCP tools active");
    pclika_bridge_run();
}
