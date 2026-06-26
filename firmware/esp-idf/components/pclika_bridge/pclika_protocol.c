/**
 * @file pclika_protocol.c
 * @brief Pclika device-side NDJSON protocol implementation
 *
 * Minimal JSON parser — avoids cJSON dependency for the hot path.
 * Uses cJSON only for nested params extraction.
 */

#include "pclika_protocol.h"
#include "cJSON.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"

static const char *TAG = "pclika_proto";


/* ── Response builders ────────────────────────────────────────────────────── */

int pclika_proto_ok(const char *id, const char *data_json, char *buf, size_t len)
{
    return snprintf(buf, len,
        "{\"id\":\"%s\",\"ok\":true,\"data\":%s}\n",
        id ? id : "",
        data_json ? data_json : "{}");
}

int pclika_proto_err(const char *id, const char *code, const char *msg,
                     char *buf, size_t len)
{
    return snprintf(buf, len,
        "{\"id\":\"%s\",\"ok\":false,\"error\":{\"code\":\"%s\",\"msg\":\"%s\"}}\n",
        id ? id : "",
        code ? code : "unknown",
        msg  ? msg  : "");
}


/* ── Command parser ───────────────────────────────────────────────────────── */

esp_err_t pclika_proto_parse(char *line, pclika_cmd_t *cmd)
{
    if (!line || !cmd) return ESP_ERR_INVALID_ARG;

    cJSON *root = cJSON_ParseWithLength(line, strlen(line));
    if (!root) {
        ESP_LOGD(TAG, "JSON parse failed: %s", line);
        return ESP_ERR_INVALID_ARG;
    }

    memset(cmd, 0, sizeof(*cmd));

    cJSON *id_item  = cJSON_GetObjectItemCaseSensitive(root, "id");
    cJSON *cmd_item = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    cJSON *par_item = cJSON_GetObjectItemCaseSensitive(root, "params");

    if (!cJSON_IsString(id_item) || !cJSON_IsString(cmd_item)) {
        cJSON_Delete(root);
        return ESP_ERR_INVALID_ARG;
    }

    strlcpy(cmd->id,  id_item->valuestring,  sizeof(cmd->id));
    strlcpy(cmd->cmd, cmd_item->valuestring, sizeof(cmd->cmd));

    /* Store serialized params for downstream extraction */
    if (par_item) {
        cmd->params_json = cJSON_PrintUnformatted(par_item);
    } else {
        cmd->params_json = strdup("{}");
    }

    cJSON_Delete(root);
    return ESP_OK;
}

void pclika_proto_free_cmd(pclika_cmd_t *cmd)
{
    if (cmd && cmd->params_json) {
        free(cmd->params_json);
        cmd->params_json = NULL;
    }
}


/* ── Parameter extractors ─────────────────────────────────────────────────── */

bool pclika_proto_get_str(const char *params_json, const char *key,
                           char *buf, size_t len)
{
    cJSON *root = cJSON_Parse(params_json);
    if (!root) return false;
    cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);
    bool found  = cJSON_IsString(item);
    if (found) strlcpy(buf, item->valuestring, len);
    cJSON_Delete(root);
    return found;
}

bool pclika_proto_get_float(const char *params_json, const char *key, float *out)
{
    cJSON *root = cJSON_Parse(params_json);
    if (!root) return false;
    cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);
    bool found  = cJSON_IsNumber(item);
    if (found) *out = (float)item->valuedouble;
    cJSON_Delete(root);
    return found;
}

bool pclika_proto_get_int(const char *params_json, const char *key, int *out)
{
    cJSON *root = cJSON_Parse(params_json);
    if (!root) return false;
    cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);
    bool found  = cJSON_IsNumber(item);
    if (found) *out = item->valueint;
    cJSON_Delete(root);
    return found;
}

bool pclika_proto_get_bool(const char *params_json, const char *key, bool *out)
{
    cJSON *root = cJSON_Parse(params_json);
    if (!root) return false;
    cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);
    bool found  = cJSON_IsBool(item);
    if (found) *out = cJSON_IsTrue(item);
    cJSON_Delete(root);
    return found;
}
