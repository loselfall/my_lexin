#include "Protocol_Http.h"

static char *response_buffer = NULL;   // 全局缓存响应体
static size_t response_buffer_len = 0; // 当前缓存长度
static char *Protocal_Http_Get_Body(void);
struct Protocal_Http
{
    esp_http_client_handle_t client;
    const char*url;
};
/// @brief http协议事件回调
/// @param evt
/// @return
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{

    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        MY_LOGI("HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        MY_LOGI("HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        MY_LOGI("HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        MY_LOGI("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        response_buffer = realloc(response_buffer, response_buffer_len + evt->data_len + 1);
        memcpy(response_buffer + response_buffer_len, evt->data, evt->data_len);
        response_buffer_len += evt->data_len;
        response_buffer[response_buffer_len] = '\0'; // 确保字符串终止
        break;
    case HTTP_EVENT_ON_FINISH:
        MY_LOGI("HTTP_EVENT_ON_FINISH, Full Response Body:\n%.*s",
                response_buffer_len, response_buffer);
        // 释放缓存
        free(response_buffer);
        response_buffer = NULL;
        response_buffer_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        MY_LOGI("HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            MY_LOGI("Last esp error code: 0x%x", err);
            MY_LOGI("Last mbedtls failure: 0x%x", mbedtls_err);
        }
        break;
    case HTTP_EVENT_REDIRECT:
        MY_LOGI("HTTP_EVENT_REDIRECT");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}

Protocal_Http_t *Protocal_Http_Init(const char *mac, const char *uuid)
{
    Protocal_Http_t *protocal_http = malloc(sizeof(Protocal_Http_t));

    // 配置https post请求 客户端
    esp_http_client_config_t config = {
        .url = XIAOZHI_OTA_URL,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .method = HTTP_METHOD_POST,
    };
    // 初始化client
    protocal_http->client = esp_http_client_init(&config);
    // 设置请求头
    esp_http_client_set_header(protocal_http->client, "Device-Id", mac);
    esp_http_client_set_header(protocal_http->client, "Client-Id", uuid);
    esp_http_client_set_header(protocal_http->client, "User-Agent", "wangwang/1.8.2");
    esp_http_client_set_header(protocal_http->client, "Accept-Language", "zh-CN");
    esp_http_client_set_header(protocal_http->client, "Content-Type", "application/json");

    // 打印请求头
    MY_LOGI("Request Headers:");
    MY_LOGI("Device-Id: %s", mac);
    MY_LOGI("Client-Id: %s", uuid);
    MY_LOGI("User-Agent: wangwang/1.8.2");
    MY_LOGI("Accept-Language: zh-CN");
    MY_LOGI("Content-Type: application/json");

    // 设置请求体
    char *body = Protocal_Http_Get_Body();
    esp_http_client_set_post_field(protocal_http->client, body, strlen(body));
    MY_LOGI("Request Body:\n%s", body);

    // 发送请求
    esp_http_client_perform(protocal_http->client);
    // 释放资源
    free(body);

    return protocal_http;
}
/// @brief
/// @param mac
/// @param uuid
/// @return 字符串使用后自行释放
static char *Protocal_Http_Get_Body()
{

    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);

    char *body_str;
    cJSON *root = cJSON_CreateObject();

    cJSON *application = cJSON_AddObjectToObject(root, "application");
    cJSON_AddStringToObject(application, "version", "1.8.2");
    cJSON_AddStringToObject(application, "elf_sha256", esp_app_get_elf_sha256_str());

    cJSON *board = cJSON_AddObjectToObject(root, "board");
    cJSON_AddStringToObject(board, "type", "bread-compact-wifi");
    cJSON_AddStringToObject(board, "name", "wangwang");
    cJSON_AddStringToObject(board, "ssid", (char*)(ap_info.ssid));
    cJSON_AddNumberToObject(board, "rssi", ap_info.rssi);
    body_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return body_str;
}