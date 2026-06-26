/**
 * @file pclika_bridge.c
 * @brief Pclika MCP Bridge — device-side command dispatcher
 *
 * Runs as a FreeRTOS task. Reads lines from UART (USB-CDC),
 * parses NDJSON commands, dispatches to runtime handlers,
 * and writes NDJSON responses.
 */

#include "pclika_bridge.h"
#include "pclika_protocol.h"
#include "pclika_system.h"
#include "pclika_sensor.h"
#include "pclika_display.h"
#include "pclika_servo.h"
#include "pclika_gpio.h"
#include "pclika_wifi.h"
#include "pclika_logging.h"

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <string.h>
#include <stdio.h>

static const char *TAG = "pclika_bridge";

static TaskHandle_t      s_task_handle = NULL;
static bool              s_running     = false;
static uart_port_t       s_uart_num    = UART_NUM_0;

/* Response + line buffers */
#define LINE_BUF_SIZE   PCLIKA_PROTO_LINE_MAX
#define RESP_BUF_SIZE   PCLIKA_PROTO_RESP_MAX

/* ── UART send helper ────────────────────────────────────────────────────── */

static void bridge_send(const char *line)
{
    uart_write_bytes(s_uart_num, line, strlen(line));
}

/* ── Command handlers ─────────────────────────────────────────────────────── */

static void handle_ping(const pclika_cmd_t *cmd)
{
    char data[128];
    snprintf(data, sizeof(data),
        "{\"pong\":true,\"version\":\"%s\",\"board\":\"%s\",\"seal\":\"PCK-MMXXVI-C4A32096\"}",
        PCLIKA_FW_VERSION, PCLIKA_BOARD_VARIANT);
    char resp[RESP_BUF_SIZE];
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_device_info(const pclika_cmd_t *cmd)
{
    char data[512];
    pclika_system_to_json(data, sizeof(data));
    char resp[RESP_BUF_SIZE];
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_firmware_version(const pclika_cmd_t *cmd)
{
    char data[256];
    snprintf(data, sizeof(data),
        "{\"version\":\"%s\",\"build_date\":\"%s\",\"platform\":\"%s\","
        "\"seal\":\"PCK-MMXXVI-C4A32096\"}",
        PCLIKA_FW_VERSION, PCLIKA_FW_BUILD_DATE, PCLIKA_PLATFORM);
    char resp[RESP_BUF_SIZE];
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_sensor_read(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    char sensor_id[PCLIKA_SENSOR_ID_MAX_LEN] = {0};
    int  channel = 0;

    if (!pclika_proto_get_str(cmd->params_json, "sensor_id", sensor_id, sizeof(sensor_id))) {
        pclika_proto_err(cmd->id, "param_invalid", "sensor_id is required", resp, sizeof(resp));
        bridge_send(resp);
        return;
    }
    pclika_proto_get_int(cmd->params_json, "channel", &channel);

    pclika_sensor_reading_t reading = {0};
    esp_err_t err = pclika_sensor_read(sensor_id, channel, &reading);
    if (err != ESP_OK || !reading.valid) {
        const char *msg = reading.error[0] ? reading.error : "sensor_read failed";
        pclika_proto_err(cmd->id, "sensor_missing", msg, resp, sizeof(resp));
        bridge_send(resp);
        return;
    }

    char data[256];
    pclika_sensor_reading_to_json(&reading, sensor_id, channel, data, sizeof(data));
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_display_text(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    char text[257] = {0};
    char display_id[PCLIKA_DISPLAY_ID_MAX_LEN] = "primary";
    int  line  = 0;
    bool clear = false;

    if (!pclika_proto_get_str(cmd->params_json, "text", text, sizeof(text))) {
        pclika_proto_err(cmd->id, "param_invalid", "text is required", resp, sizeof(resp));
        bridge_send(resp);
        return;
    }
    pclika_proto_get_int(cmd->params_json,  "line",       &line);
    pclika_proto_get_bool(cmd->params_json, "clear",      &clear);
    pclika_proto_get_str(cmd->params_json,  "display_id", display_id, sizeof(display_id));

    esp_err_t err = pclika_display_text(display_id, text, line, clear);
    char data[64];
    pclika_display_result_to_json(err == ESP_OK, err != ESP_OK ? "display_error" : NULL,
                                   data, sizeof(data));
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_servo_move(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    float angle = -1.0f;
    int   channel = 0;
    int   speed   = 100;

    if (!pclika_proto_get_float(cmd->params_json, "angle", &angle) || angle < 0) {
        pclika_proto_err(cmd->id, "param_invalid", "angle is required (0–180)", resp, sizeof(resp));
        bridge_send(resp);
        return;
    }
    pclika_proto_get_int(cmd->params_json, "channel", &channel);
    pclika_proto_get_int(cmd->params_json, "speed",   &speed);

    esp_err_t err = pclika_servo_move(channel, angle, speed);
    char data[128];
    pclika_servo_result_to_json(channel, angle, err == ESP_OK,
                                 err != ESP_OK ? "servo_error" : NULL,
                                 data, sizeof(data));
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_serial_log_read(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    int  n = 50;
    pclika_proto_get_int(cmd->params_json, "lines", &n);
    if (n < 1)  n = 1;
    if (n > 200) n = 200;

    const char *lines[200];
    int count = pclika_logging_get_lines(lines, n);

    /* Build JSON array of log lines */
    char data[PCLIKA_PROTO_RESP_MAX - 64];
    int  pos  = 0;
    pos += snprintf(data + pos, sizeof(data) - pos, "{\"lines\":[");
    for (int i = 0; i < count && pos < (int)sizeof(data) - 64; i++) {
        if (i > 0) pos += snprintf(data + pos, sizeof(data) - pos, ",");
        pos += snprintf(data + pos, sizeof(data) - pos, "\"%s\"", lines[i]);
    }
    pos += snprintf(data + pos, sizeof(data) - pos, "],\"count\":%d}", count);

    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_led_control(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    char state[16] = "off";
    pclika_proto_get_str(cmd->params_json, "state", state, sizeof(state));
    /* LED driver integration deferred — acknowledge command */
    char data[64];
    snprintf(data, sizeof(data), "{\"state\":\"%s\",\"ok\":true}", state);
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_button_read(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    /* Boot button is GPIO0 on ESP32-S3 */
    int level = 0;
    pclika_gpio_read(0, PCLIKA_PULL_UP, &level);
    char data[64];
    snprintf(data, sizeof(data), "{\"pressed\":%s,\"button_id\":\"boot\"}",
             level == 0 ? "true" : "false");  /* active low */
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_wifi_scan(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    int max_results = 20;
    pclika_proto_get_int(cmd->params_json, "max_results", &max_results);
    if (max_results < 1)  max_results = 1;
    if (max_results > 50) max_results = 50;

    esp_err_t err = pclika_wifi_scan(max_results);
    if (err != ESP_OK) {
        pclika_proto_err(cmd->id, "wifi_error", "scan failed", resp, sizeof(resp));
        bridge_send(resp);
        return;
    }

    pclika_wifi_ap_t aps[50];
    int count = pclika_wifi_get_scan_results(aps, max_results);
    char data[RESP_BUF_SIZE - 64];
    pclika_wifi_scan_to_json(aps, count, data, sizeof(data));
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_gpio_read(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    int pin = -1;
    char pull_str[8] = "none";
    if (!pclika_proto_get_int(cmd->params_json, "pin", &pin) || pin < 0) {
        pclika_proto_err(cmd->id, "param_invalid", "pin is required", resp, sizeof(resp));
        bridge_send(resp);
        return;
    }
    pclika_proto_get_str(cmd->params_json, "pull", pull_str, sizeof(pull_str));
    pclika_pull_t pull = PCLIKA_PULL_NONE;
    if (strcmp(pull_str, "up")   == 0) pull = PCLIKA_PULL_UP;
    if (strcmp(pull_str, "down") == 0) pull = PCLIKA_PULL_DOWN;

    int level = 0;
    pclika_gpio_read(pin, pull, &level);
    char data[64];
    pclika_gpio_read_to_json(pin, level, data, sizeof(data));
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

static void handle_gpio_write(const pclika_cmd_t *cmd)
{
    char resp[RESP_BUF_SIZE];
    int pin = -1, level = -1;
    if (!pclika_proto_get_int(cmd->params_json, "pin",   &pin)   || pin < 0 ||
        !pclika_proto_get_int(cmd->params_json, "level", &level) || (level != 0 && level != 1)) {
        pclika_proto_err(cmd->id, "param_invalid", "pin and level (0 or 1) are required",
                          resp, sizeof(resp));
        bridge_send(resp);
        return;
    }
    esp_err_t err = pclika_gpio_write(pin, level);
    char data[64];
    pclika_gpio_write_to_json(pin, level, err == ESP_OK, data, sizeof(data));
    pclika_proto_ok(cmd->id, data, resp, sizeof(resp));
    bridge_send(resp);
}

/* ── Command dispatch table ──────────────────────────────────────────────── */

typedef void (*cmd_handler_fn)(const pclika_cmd_t *);

typedef struct {
    const char     *cmd;
    cmd_handler_fn  handler;
} cmd_entry_t;

static const cmd_entry_t CMD_TABLE[] = {
    { "ping",             handle_ping            },
    { "device_info",      handle_device_info     },
    { "firmware_version", handle_firmware_version},
    { "sensor_read",      handle_sensor_read     },
    { "display_text",     handle_display_text    },
    { "servo_move",       handle_servo_move      },
    { "serial_log_read",  handle_serial_log_read },
    { "led_control",      handle_led_control     },
    { "button_read",      handle_button_read     },
    { "wifi_scan",        handle_wifi_scan       },
    { "gpio_read",        handle_gpio_read       },
    { "gpio_write",       handle_gpio_write      },
    { NULL, NULL }
};

static void dispatch(const pclika_cmd_t *cmd)
{
    for (int i = 0; CMD_TABLE[i].cmd != NULL; i++) {
        if (strcmp(cmd->cmd, CMD_TABLE[i].cmd) == 0) {
            CMD_TABLE[i].handler(cmd);
            return;
        }
    }
    /* Unknown command */
    char resp[256];
    char msg[96];
    snprintf(msg, sizeof(msg), "unknown command: %s", cmd->cmd);
    pclika_proto_err(cmd->id, "tool_not_supported", msg, resp, sizeof(resp));
    bridge_send(resp);
}

/* ── Bridge task ─────────────────────────────────────────────────────────── */

static void bridge_task(void *arg)
{
    static char line_buf[LINE_BUF_SIZE];
    int  pos   = 0;
    uint8_t ch = 0;

    ESP_LOGI(TAG, "Bridge task started on UART%d", (int)s_uart_num);

    while (s_running) {
        int n = uart_read_bytes(s_uart_num, &ch, 1, pdMS_TO_TICKS(10));
        if (n <= 0) continue;

        if (ch == '\n' || ch == '\r') {
            if (pos > 0) {
                line_buf[pos] = '\0';
                ESP_LOGD(TAG, "→ %s", line_buf);

                pclika_cmd_t cmd = {0};
                if (pclika_proto_parse(line_buf, &cmd) == ESP_OK) {
                    dispatch(&cmd);
                    if (cmd.params_json) free(cmd.params_json);
                } else {
                    char err_resp[256];
                    pclika_proto_err("", "parse_error", "malformed JSON frame",
                                     err_resp, sizeof(err_resp));
                    bridge_send(err_resp);
                }
                pos = 0;
            }
        } else {
            if (pos < LINE_BUF_SIZE - 1) {
                line_buf[pos++] = (char)ch;
            } else {
                /* Line too long — discard */
                pos = 0;
                ESP_LOGW(TAG, "Input line too long — discarded");
            }
        }
    }

    ESP_LOGI(TAG, "Bridge task stopped");
    vTaskDelete(NULL);
}

/* ── Public API ──────────────────────────────────────────────────────────── */

esp_err_t pclika_bridge_start(const pclika_bridge_config_t *cfg)
{
    pclika_bridge_config_t c = PCLIKA_BRIDGE_CONFIG_DEFAULT();
    if (cfg) c = *cfg;

    s_uart_num = c.uart_num;

    /* Configure UART (USB-CDC on ESP32-S3 uses UART_NUM_0 automatically) */
    uart_config_t uart_cfg = {
        .baud_rate  = c.baud_rate,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(c.uart_num, c.rx_buf_size, c.tx_buf_size, 0, NULL, 0);
    uart_param_config(c.uart_num, &uart_cfg);

    s_running = true;
    BaseType_t rc = xTaskCreate(bridge_task, "pclika_bridge",
                                 c.stack_size, NULL, c.priority, &s_task_handle);
    if (rc != pdPASS) {
        s_running = false;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "pclika-bridge v%s started | seal: PCK-MMXXVI-C4A32096", PCLIKA_FW_VERSION);
    return ESP_OK;
}

esp_err_t pclika_bridge_stop(void)
{
    s_running = false;
    if (s_task_handle) {
        vTaskDelay(pdMS_TO_TICKS(100));
        s_task_handle = NULL;
    }
    uart_driver_delete(s_uart_num);
    return ESP_OK;
}

bool pclika_bridge_is_running(void)
{
    return s_running;
}
