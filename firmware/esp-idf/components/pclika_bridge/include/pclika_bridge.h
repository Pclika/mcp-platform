#pragma once
/**
 * @file pclika_bridge.h
 * @brief Pclika MCP Bridge — device-side command dispatcher
 *
 * Reads NDJSON command lines from UART, dispatches to runtime handlers,
 * and writes NDJSON response lines back.
 */

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Bridge configuration. */
typedef struct {
    int    uart_num;       /**< UART port number (default: UART_NUM_0 = USB-CDC) */
    int    baud_rate;      /**< Baud rate (default: 115200) */
    int    rx_buf_size;    /**< RX ring buffer (default: 2048) */
    int    tx_buf_size;    /**< TX ring buffer (default: 2048) */
    int    stack_size;     /**< Bridge task stack (default: 8192) */
    int    priority;       /**< Bridge task priority (default: 5) */
} pclika_bridge_config_t;

/** Default bridge configuration. */
#define PCLIKA_BRIDGE_CONFIG_DEFAULT() { \
    .uart_num    = 0,     \
    .baud_rate   = 115200,\
    .rx_buf_size = 2048,  \
    .tx_buf_size = 2048,  \
    .stack_size  = 8192,  \
    .priority    = 5,     \
}

/**
 * @brief Initialize and start the bridge task.
 *        Must be called after all runtime modules are initialized.
 * @param cfg  Bridge configuration (NULL = use defaults)
 */
esp_err_t pclika_bridge_start(const pclika_bridge_config_t *cfg);

/**
 * @brief Stop the bridge task and free resources.
 */
esp_err_t pclika_bridge_stop(void);

/**
 * @brief Check if bridge is running.
 */
bool pclika_bridge_is_running(void);

#ifdef __cplusplus
}
#endif
