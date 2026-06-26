#pragma once
/**
 * @file pclika_wifi.h
 * @brief Pclika Runtime — Wi-Fi module
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_WIFI_SSID_MAX_LEN  33
#define PCLIKA_WIFI_SCAN_MAX      20

typedef struct {
    char    ssid[PCLIKA_WIFI_SSID_MAX_LEN];
    int8_t  rssi;
    uint8_t channel;
    uint8_t authmode;   /**< esp_wifi_types authmode value */
    uint8_t bssid[6];
} pclika_wifi_ap_t;

typedef enum {
    PCLIKA_WIFI_STATE_IDLE       = 0,
    PCLIKA_WIFI_STATE_CONNECTING,
    PCLIKA_WIFI_STATE_CONNECTED,
    PCLIKA_WIFI_STATE_FAILED,
    PCLIKA_WIFI_STATE_DISCONNECTED,
} pclika_wifi_state_t;

/** Initialize Wi-Fi subsystem (station mode). */
esp_err_t pclika_wifi_init(void);

/** Scan for nearby APs. Results stored internally. */
esp_err_t pclika_wifi_scan(int max_results);

/** Get scan results. Returns number of APs found. */
int pclika_wifi_get_scan_results(pclika_wifi_ap_t *results, int max);

/** Connect to an AP. Credentials stored in NVS. */
esp_err_t pclika_wifi_connect(const char *ssid, const char *password);

/** Disconnect from current AP. */
esp_err_t pclika_wifi_disconnect(void);

/** Get current connection state. */
pclika_wifi_state_t pclika_wifi_get_state(void);

/** Serialize scan results to JSON. Returns bytes written. */
int pclika_wifi_scan_to_json(const pclika_wifi_ap_t *aps, int count,
                              char *buf, size_t len);

#ifdef __cplusplus
}
#endif
