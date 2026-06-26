/**
 * @file pclika_system.c
 * @brief Pclika platform system layer — board identity, capabilities, heap info
 */

#include "pclika_system.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "pclika_sys";

/* ── Internal state ─────────────────────────────────────────────────────── */

static char   s_board_id[32]   = {0};
static bool   s_initialized    = false;

#define MAX_CAPS 16
static struct {
    char  key[32];
    bool  value;
} s_caps[MAX_CAPS];
static int s_cap_count = 0;

/* ── Init ────────────────────────────────────────────────────────────────── */

esp_err_t pclika_system_init(void)
{
    if (s_initialized) return ESP_OK;

    /* Build board_id from MAC address */
    uint8_t mac[6] = {0};
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(s_board_id, sizeof(s_board_id),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /* Default capabilities */
    pclika_system_set_cap("has_sensor",  false);
    pclika_system_set_cap("has_display", false);
    pclika_system_set_cap("has_servo",   false);
    pclika_system_set_cap("has_wifi",    false);

    s_initialized = true;
    ESP_LOGI(TAG, "board_id=%s", s_board_id);
    return ESP_OK;
}

/* ── Board ID ────────────────────────────────────────────────────────────── */

const char *pclika_system_board_id(void)
{
    return s_board_id;
}

/* ── Capabilities ────────────────────────────────────────────────────────── */

esp_err_t pclika_system_set_cap(const char *key, bool value)
{
    if (!key) return ESP_ERR_INVALID_ARG;

    /* Update existing */
    for (int i = 0; i < s_cap_count; i++) {
        if (strcmp(s_caps[i].key, key) == 0) {
            s_caps[i].value = value;
            return ESP_OK;
        }
    }

    /* Add new */
    if (s_cap_count >= MAX_CAPS) return ESP_ERR_NO_MEM;
    snprintf(s_caps[s_cap_count].key, sizeof(s_caps[0].key), "%s", key);
    s_caps[s_cap_count].value = value;
    s_cap_count++;
    return ESP_OK;
}

bool pclika_system_get_cap(const char *key)
{
    for (int i = 0; i < s_cap_count; i++) {
        if (strcmp(s_caps[i].key, key) == 0) return s_caps[i].value;
    }
    return false;
}

esp_err_t pclika_system_get_caps(pclika_caps_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;
    out->has_sensor  = pclika_system_get_cap("has_sensor");
    out->has_display = pclika_system_get_cap("has_display");
    out->has_servo   = pclika_system_get_cap("has_servo");
    out->has_wifi    = pclika_system_get_cap("has_wifi");
    return ESP_OK;
}

/* ── Heap info ───────────────────────────────────────────────────────────── */

esp_err_t pclika_system_heap_info(pclika_heap_info_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;
    out->free_bytes      = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    out->total_bytes     = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    out->min_free_bytes  = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    return ESP_OK;
}

/* ── JSON serialisation ──────────────────────────────────────────────────── */

char *pclika_system_info_json(void)
{
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON_AddStringToObject(root, "board_id",  s_board_id);
    cJSON_AddStringToObject(root, "firmware",  PCLIKA_FIRMWARE_VERSION);
    cJSON_AddStringToObject(root, "platform",  "esp32s3");
    cJSON_AddStringToObject(root, "idf",       IDF_VER);
    cJSON_AddStringToObject(root, "seal",      PCLIKA_PLATFORM_SEAL);
    cJSON_AddStringToObject(root, "build_date", __DATE__);
    cJSON_AddStringToObject(root, "build_time", __TIME__);

    cJSON *caps = cJSON_CreateObject();
    for (int i = 0; i < s_cap_count; i++) {
        cJSON_AddBoolToObject(caps, s_caps[i].key, s_caps[i].value);
    }
    cJSON_AddItemToObject(root, "capabilities", caps);

    pclika_heap_info_t heap;
    pclika_system_heap_info(&heap);
    cJSON *heap_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(heap_obj, "free",      heap.free_bytes);
    cJSON_AddNumberToObject(heap_obj, "total",     heap.total_bytes);
    cJSON_AddNumberToObject(heap_obj, "min_free",  heap.min_free_bytes);
    cJSON_AddItemToObject(root, "heap", heap_obj);

    char *out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return out;  /* caller must free() */
}
