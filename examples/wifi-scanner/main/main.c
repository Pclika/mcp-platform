/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Pclika  https://pclika.com
 *
 * wifi-scanner — MCP Example
 * PCK-MMXXVI-C4A32096
 *
 * Demonstrates the wifi_scan MCP tool.
 * Scans for nearby Wi-Fi APs and returns results as structured JSON.
 *
 * MCP tool exposed: wifi_scan
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "cJSON.h"

#include "pclika_bridge.h"
#include "pclika_runtime.h"

#define TAG "wifi-scanner"
#define MAX_AP 20

/* -------- Wi-Fi scan helper -------- */

typedef struct {
    char     ssid[33];
    int8_t   rssi;
    uint8_t  channel;
    char     bssid[18];
    uint8_t  authmode;
} ap_record_t;

static const char *authmode_str(uint8_t mode)
{
    switch (mode) {
        case WIFI_AUTH_OPEN:            return "OPEN";
        case WIFI_AUTH_WEP:             return "WEP";
        case WIFI_AUTH_WPA_PSK:         return "WPA";
        case WIFI_AUTH_WPA2_PSK:        return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2";
        case WIFI_AUTH_WPA3_PSK:        return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/WPA3";
        default:                        return "UNKNOWN";
    }
}

static esp_err_t do_wifi_scan(uint8_t max_results,
                               ap_record_t *records, uint16_t *count,
                               uint32_t *scan_time_ms)
{
    wifi_scan_config_t scan_cfg = {
        .ssid        = NULL,
        .bssid       = NULL,
        .channel     = 0,
        .show_hidden = true,
        .scan_type   = WIFI_SCAN_TYPE_ACTIVE,
    };

    int64_t t0 = esp_timer_get_time();
    esp_err_t ret = esp_wifi_scan_start(&scan_cfg, true);  /* blocking */
    *scan_time_ms = (uint32_t)((esp_timer_get_time() - t0) / 1000);

    if (ret != ESP_OK) return ret;

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    if (ap_count > max_results) ap_count = max_results;

    wifi_ap_record_t raw[MAX_AP];
    uint16_t n = ap_count;
    ret = esp_wifi_scan_get_ap_records(&n, raw);
    if (ret != ESP_OK) return ret;

    for (uint16_t i = 0; i < n; i++) {
        memcpy(records[i].ssid, raw[i].ssid, 33);
        records[i].ssid[32]  = '\0';
        records[i].rssi      = raw[i].rssi;
        records[i].channel   = raw[i].primary;
        records[i].authmode  = raw[i].authmode;
        snprintf(records[i].bssid, sizeof(records[i].bssid),
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 raw[i].bssid[0], raw[i].bssid[1], raw[i].bssid[2],
                 raw[i].bssid[3], raw[i].bssid[4], raw[i].bssid[5]);
    }
    *count = n;
    return ESP_OK;
}

/* -------- MCP tool handler -------- */

static void tool_wifi_scan(const cJSON *params, cJSON *result)
{
    int max_results = 20;
    cJSON *j = cJSON_GetObjectItem(params, "max_results");
    if (cJSON_IsNumber(j) && j->valueint > 0 && j->valueint <= MAX_AP) {
        max_results = j->valueint;
    }

    ap_record_t records[MAX_AP];
    uint16_t count = 0;
    uint32_t scan_time_ms = 0;

    esp_err_t ret = do_wifi_scan((uint8_t)max_results, records, &count, &scan_time_ms);
    if (ret != ESP_OK) {
        cJSON_AddStringToObject(result, "error", esp_err_to_name(ret));
        return;
    }

    cJSON *networks = cJSON_AddArrayToObject(result, "networks");
    for (uint16_t i = 0; i < count; i++) {
        cJSON *ap = cJSON_CreateObject();
        cJSON_AddStringToObject(ap, "ssid",     records[i].ssid);
        cJSON_AddNumberToObject(ap, "rssi",     records[i].rssi);
        cJSON_AddNumberToObject(ap, "channel",  records[i].channel);
        cJSON_AddStringToObject(ap, "security", authmode_str(records[i].authmode));
        cJSON_AddStringToObject(ap, "bssid",    records[i].bssid);
        cJSON_AddItemToArray(networks, ap);
    }
    cJSON_AddNumberToObject(result, "count",        count);
    cJSON_AddNumberToObject(result, "scan_time_ms", scan_time_ms);

    ESP_LOGI(TAG, "scan complete: %u APs in %lu ms", count, (unsigned long)scan_time_ms);
}

/* -------- Wi-Fi init (station mode, no AP connection) -------- */

static void wifi_init_scan_mode(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi STA started (scan-only mode)");
}

/* -------- app_main -------- */

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    pclika_runtime_config_t rt_cfg = PCLIKA_RUNTIME_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(pclika_runtime_init(&rt_cfg));

    wifi_init_scan_mode();

    /* Register MCP tools */
    pclika_bridge_register_tool("wifi_scan", tool_wifi_scan);

    ESP_LOGI(TAG, "wifi-scanner ready — MCP tool: wifi_scan");
    pclika_bridge_run();  /* blocks, drives JSON-RPC over UART */
}
