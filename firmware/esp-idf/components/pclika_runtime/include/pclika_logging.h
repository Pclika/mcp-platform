#pragma once
/**
 * @file pclika_logging.h
 * @brief Pclika Runtime — Logging module
 *
 * Wraps ESP-IDF logging and maintains a circular ring buffer
 * that the MCP bridge can read via serial_log_read.
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_LOG_TAG          "PCLIKA"
#define PCLIKA_LOG_BUF_LINES    200     /**< Ring buffer size in lines */
#define PCLIKA_LOG_LINE_MAX     256     /**< Max chars per line */

/** Initialize logging. Install custom vprintf to capture log lines. */
esp_err_t pclika_logging_init(void);

/**
 * @brief Read the last N lines from the log ring buffer.
 * @param[out] lines  Array of string pointers (caller must not free)
 * @param      n      Max lines to return
 * @return Number of lines returned
 */
int pclika_logging_get_lines(const char **lines, int n);

/** Log a message at INFO level and add to ring buffer. */
void pclika_log_info(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** Log a message at WARN level. */
void pclika_log_warn(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** Log a message at ERROR level. */
void pclika_log_error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif
