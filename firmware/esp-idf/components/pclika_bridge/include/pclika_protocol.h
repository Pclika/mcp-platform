#pragma once
/**
 * @file pclika_protocol.h
 * @brief Pclika device-side NDJSON bridge protocol
 *
 * Frame format (host → device, one line):
 *   {"id":"<uuid>","cmd":"<tool>","params":{...}}\n
 *
 * Frame format (device → host, one line):
 *   {"id":"<uuid>","ok":true,"data":{...}}\n
 *   {"id":"<uuid>","ok":false,"error":{"code":"<code>","msg":"<message>"}}\n
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCLIKA_PROTO_LINE_MAX   1024    /**< Max bytes per input line */
#define PCLIKA_PROTO_ID_MAX     64      /**< Max UUID string length */
#define PCLIKA_PROTO_CMD_MAX    64      /**< Max command name length */
#define PCLIKA_PROTO_RESP_MAX   2048    /**< Max response line length */

/* ── Parsed command frame ─────────────────────────────────────────────────── */

typedef struct {
    char    id[PCLIKA_PROTO_ID_MAX];
    char    cmd[PCLIKA_PROTO_CMD_MAX];
    char   *params_json;    /**< Points into input buffer; do not free */
} pclika_cmd_t;

/* ── Response builders ────────────────────────────────────────────────────── */

/**
 * @brief Build a success response frame.
 * @param id        Request ID (echoed back)
 * @param data_json JSON string for the "data" field
 * @param[out] buf  Output buffer
 * @param len       Buffer length
 * @return bytes written (excluding null terminator)
 */
int pclika_proto_ok(const char *id, const char *data_json,
                    char *buf, size_t len);

/**
 * @brief Build an error response frame.
 * @param id        Request ID
 * @param code      Error code string (e.g. "tool_not_supported")
 * @param msg       Human-readable error message
 * @param[out] buf  Output buffer
 * @param len       Buffer length
 * @return bytes written
 */
int pclika_proto_err(const char *id, const char *code, const char *msg,
                     char *buf, size_t len);

/**
 * @brief Parse a NDJSON command line into a pclika_cmd_t.
 * @param line   Input line (null-terminated, will be modified in place)
 * @param[out] cmd  Parsed command
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if malformed
 */
esp_err_t pclika_proto_parse(char *line, pclika_cmd_t *cmd);

/**
 * @brief Extract a string parameter from the params JSON.
 * @param params_json  Params JSON string
 * @param key          Parameter key
 * @param[out] buf     Output buffer
 * @param len          Buffer length
 * @return true if found and extracted
 */
bool pclika_proto_get_str(const char *params_json, const char *key,
                           char *buf, size_t len);

/**
 * @brief Extract a float parameter from the params JSON.
 * @return true if found
 */
bool pclika_proto_get_float(const char *params_json, const char *key,
                             float *out);

/**
 * @brief Extract an int parameter from the params JSON.
 * @return true if found
 */
bool pclika_proto_get_int(const char *params_json, const char *key,
                           int *out);

/**
 * @brief Extract a bool parameter from the params JSON.
 * @return true if found
 */
bool pclika_proto_get_bool(const char *params_json, const char *key,
                            bool *out);

#ifdef __cplusplus
}
#endif
