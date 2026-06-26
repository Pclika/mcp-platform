/**
 * @file pclika_wifi.c
 * @brief Pclika Wi-Fi — scan + connect + status, event-driven
 */

#include "pclika_wifi.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "pclika_wifi";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_events = NULL;
static pclika_wifi_state_t s_state = PCLIKA_WIFI_DISCONNECTED;
static char s_connected_ssid[33] = {0};
static char s_ip[16] = {0};
static bool s_initialized = false;

/* ── Event handler ───────────────────────────────────────────────────────── */

static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    if (base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            s_state = PCLIKA_WIFI_DISCONNECTED;
            memset(s_ip, 0, sizeof(s_ip));
            xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);
            ESP_LOGW(TAG, "disconnected");
        }
    } else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ev = (ip_event_got_ip_t *)event_data;
        snprintf(s_ip, sizeof(s_ip), IPSTR, IP2STR(&ev->ip_info.ip));
        s_state = PCLIKA_WIFI_CONNECTED;
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "connected, IP=%s", s_ip);
    }
}

/* ── Init ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_wifi_init(void)
{
    if (s_initialized) return ESP_OK;

    s_wifi_events = xEventGroupCreate();
    if (!s_wifi_events) return ESP_ERR_NO_MEM;

    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t inst_any_id, inst_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &wifi_event_handler, NULL, &inst_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler, NULL, &inst_got_ip);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_initialized = true;
    s_state = PCLIKA_WIFI_DISCONNECTED;
    ESP_LOGI(TAG, "wifi init OK");
    return ESP_OK;
}

/* ── Scan ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_wifi_scan(pclika_wifi_ap_t *results, int max_results, int *found_out)
{
    if (!results || max_results <= 0 || !found_out) return ESP_ERR_INVALID_ARG;
    if (!s_initialized) return ESP_ERR_INVALID_STATE;

    wifi_scan_config_t scan_cfg = {
        .ssid        = NULL,
        .bssid       = NULL,
        .channel     = 0,
        .show_hidden = false,
    };
    esp_err_t err = esp_wifi_scan_start(&scan_cfg, true);  /* blocking */
    if (err != ESP_OK) return err;

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);

    uint16_t n = (ap_count < (uint16_t)max_results) ? ap_count : (uint16_t)max_results;
    wifi_ap_record_t *records = malloc(n * sizeof(wifi_ap_record_t));
    if (!records) return ESP_ERR_NO_MEM;

    esp_wifi_scan_get_ap_records(&n, records);

    for (int i = 0; i < n; i++) {
        snprintf(results[i].ssid, sizeof(results[i].ssid), "%s", (char*)records[i].ssid);
        results[i].rssi    = records[i].rssi;
        results[i].channel = records[i].primary;
        results[i].open    = (records[i].authmode == WIFI_AUTH_OPEN);
    }

    free(records);
    *found_out = n;
    return ESP_OK;
}

/* ── Connect ─────────────────────────────────────────────────────────────── */

esp_err_t pclika_wifi_connect(const char *ssid, const char *password, int timeout_ms)
{
    if (!ssid) return ESP_ERR_INVALID_ARG;
    if (!s_initialized) return ESP_ERR_INVALID_STATE;

    wifi_config_t cfg = {0};
    snprintf((char*)cfg.sta.ssid,     sizeof(cfg.sta.ssid),     "%s", ssid);
    snprintf((char*)cfg.sta.password, sizeof(cfg.sta.password),  "%s", password ? password : "");

    esp_wifi_disconnect();
    esp_wifi_set_config(WIFI_IF_STA, &cfg);

    xEventGroupClearBits(s_wifi_events, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    s_state = PCLIKA_WIFI_CONNECTING;
    esp_wifi_connect();

    EventBits_t bits = xEventGroupWaitBits(s_wifi_events,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE,
                                           pdMS_TO_TICKS(timeout_ms));
    if (bits & WIFI_CONNECTED_BIT) {
        snprintf(s_connected_ssid, sizeof(s_connected_ssid), "%s", ssid);
        return ESP_OK;
    }

    s_state = PCLIKA_WIFI_DISCONNECTED;
    return ESP_ERR_TIMEOUT;
}

/* ── Status ──────────────────────────────────────────────────────────────── */

pclika_wifi_state_t pclika_wifi_state(void)     { return s_state; }
const char *pclika_wifi_ssid(void)              { return s_connected_ssid; }
const char *pclika_wifi_ip(void)                { return s_ip; }
