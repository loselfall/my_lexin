#include "Protocol_Websocket.h"
struct ProtocolWebsocket
{
    esp_websocket_client_handle_t client;
    StrCallback jsoncallback;
    StrCallback opuscallback;
    void (*finishcallback)(void);
    char *sessionid;
    size_t id_len;
};

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{

    ProtocolWebsocket_t *protocol_websocket = (ProtocolWebsocket_t *)handler_args;
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_BEGIN:
        MY_LOGI("WEBSOCKET_EVENT_BEGIN"
                "data[%d]{%.*s}\r\n",
                data->data_len, data->data_len, (char *)data->data_ptr);
        break;
    case WEBSOCKET_EVENT_CONNECTED:
        MY_LOGI("WEBSOCKET_EVENT_CONNECTED"
                "data[%d]{%.*s}\r\n",
                data->data_len, data->data_len, (char *)data->data_ptr);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        MY_LOGI("WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        // 接收到Json文本帧
        if (data->op_code == 0x1)
        {
            if (protocol_websocket->jsoncallback != NULL)
            {
                protocol_websocket->jsoncallback(protocol_websocket, data->data_ptr, data->data_len);
            }
            else
            {
                MY_LOGE("json回调未设置");
            }
        }
        // 接收到2进制数据流
        else if (data->op_code == 0x2)
        {
            if (protocol_websocket->opuscallback != NULL)
            {
                protocol_websocket->opuscallback(protocol_websocket, data->data_ptr, data->data_len);
            }
            else
            {
                MY_LOGE("opus回调未设置");
            }
        }
        else if (data->op_code == 0x0a)
        {
            MY_LOGI("PingPong");
        }
        else if (data->op_code == 0x08 && data->data_len == 2)
        {
            MY_LOGW("Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
        }
        else
        {
            MY_LOGW("opcode[%d],data[%d]{%.*s}\r\n", 256 * data->data_ptr[0] + data->data_ptr[1], data->data_len, data->data_len, (char *)data->data_ptr);
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        MY_LOGI("WEBSOCKET_EVENT_ERROR");
        if (protocol_websocket->finishcallback != NULL)
        {
            protocol_websocket->finishcallback();
        }
        break;
    case WEBSOCKET_EVENT_FINISH:
        MY_LOGI("WEBSOCKET_EVENT_FINISH");
        if (protocol_websocket->finishcallback != NULL)
        {
            protocol_websocket->finishcallback();
        }
        break;
    }
}

ProtocolWebsocket_t *Protocol_Websocket_Init(const char *url, const char *mac, const char *uuid)
{
    // 创建自定义句柄
    ProtocolWebsocket_t *protocol_websocket = malloc(sizeof(ProtocolWebsocket_t));
    protocol_websocket->id_len = 0;
    protocol_websocket->sessionid = NULL;
    // 设置websocket配置信息
    esp_websocket_client_config_t websocket_cfg = {
        .uri = url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size = 2048,
        .transport = WEBSOCKET_TRANSPORT_OVER_SSL,
        .network_timeout_ms = 5000,
        .reconnect_timeout_ms = 5000,
    };
    // 初始化客户端
    protocol_websocket->client = esp_websocket_client_init(&websocket_cfg);

    // 追加请求头
    esp_websocket_client_append_header(protocol_websocket->client, "Authorization", "Bearer <access_token>");
    esp_websocket_client_append_header(protocol_websocket->client, "Protocol-Version", "1");
    esp_websocket_client_append_header(protocol_websocket->client, "Device-Id", mac);
    esp_websocket_client_append_header(protocol_websocket->client, "Client-Id", uuid);
    // 注册回调函数
    esp_websocket_register_events(protocol_websocket->client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)protocol_websocket);
    // 返回自定义句柄
    return protocol_websocket;
}

void Protocol_Websocket_Start(ProtocolWebsocket_t *protocol_websocket)
{
    // 启动客户端
    if (protocol_websocket->client != NULL && esp_websocket_client_is_connected(protocol_websocket->client) == false)
    {
        esp_websocket_client_start(protocol_websocket->client);
        while (esp_websocket_client_is_connected(protocol_websocket->client) == false)
        {
            MY_LOGW("等待socket连接");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

bool Protocol_Websocket_Is_Connect(ProtocolWebsocket_t *protocol_websocket)
{
    if (esp_websocket_client_is_connected(protocol_websocket->client))
    {
        return true;
    }
    Com_State_Change(UNCONNECT);
    return false;
}

void Protocol_Websocket_Close(ProtocolWebsocket_t *protocol_websocket)
{
    // 关闭客户端
    esp_websocket_client_close(protocol_websocket->client, portMAX_DELAY);
}

void Protocol_Websocket_CB_register(ProtocolWebsocket_t *protocol_websocket, StrCallback jsoncb, StrCallback opuscb, void (*finishcb)(void))
{
    protocol_websocket->jsoncallback = jsoncb;
    protocol_websocket->opuscallback = opuscb;
    protocol_websocket->finishcallback = finishcb;
}

void Protocol_Websocket_Set_Session(ProtocolWebsocket_t *protocol_websocket, char *session_id, size_t len)
{
    protocol_websocket->sessionid = session_id;
    protocol_websocket->id_len = len;
}

char *Protocol_Websocket_Get_Session(ProtocolWebsocket_t *protocol_websocket, size_t *len)
{
    *len = protocol_websocket->id_len;
    return protocol_websocket->sessionid;
}

void Protocol_Websocket_Send_Hello(ProtocolWebsocket_t *protocol_websocket)
{
    if (Protocol_Websocket_Is_Connect(protocol_websocket))
    {
        const char *data = "{\"type\":\"hello\",\"version\":1,\"transport\":\"websocket\",\"features\":{\"mcp\":true},\"audio_params\":{\"format\":\"opus\",\"sample_rate\":16000,\"channels\":1,\"frame_duration\":60}}";
        int len = strlen(data);
        esp_websocket_client_send_text(protocol_websocket->client, data, len, portMAX_DELAY);
        MY_LOGI("Hello包发送完成");
    }
    else
    {
        MY_LOGE("客户端未连接,Hello包未发送");
    }
}

void Protocol_Websocket_Send_Wakeup_Word(ProtocolWebsocket_t *protocol_websocket)
{

    if (Protocol_Websocket_Is_Connect(protocol_websocket))
    {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "session_id", protocol_websocket->sessionid);
        cJSON_AddStringToObject(root, "type", "listen");
        cJSON_AddStringToObject(root, "state", "detect");
        cJSON_AddStringToObject(root, "text", "你好乐鑫");
        char *data = cJSON_PrintUnformatted(root);
        int len = strlen(data);
        esp_websocket_client_send_text(protocol_websocket->client, data, len, portMAX_DELAY);
        cJSON_free(data);
        MY_LOGI("唤醒包发送完成");
    }
    else
    {
        MY_LOGE("客户端未连接,唤醒包未发送");
    }
}

void Protocol_Websocket_Send_Listen_Start(ProtocolWebsocket_t *protocol_websocket)
{
    if (Protocol_Websocket_Is_Connect(protocol_websocket))
    {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "session_id", protocol_websocket->sessionid);
        cJSON_AddStringToObject(root, "type", "listen");
        cJSON_AddStringToObject(root, "state", "start");
        cJSON_AddStringToObject(root, "mode", "auto");
        char *data = cJSON_PrintUnformatted(root);
        int len = strlen(data);
        esp_websocket_client_send_text(protocol_websocket->client, data, len, portMAX_DELAY);
        cJSON_free(data);
        MY_LOGI("开始侦听包发送完成");
    }
    else
    {
        MY_LOGE("客户端未连接,开始侦听包未发送");
    }
}

void Protocol_Websocket_Send_Listen_Stop(ProtocolWebsocket_t *protocol_websocket)
{
    if (Protocol_Websocket_Is_Connect(protocol_websocket))
    {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "session_id", protocol_websocket->sessionid);
        cJSON_AddStringToObject(root, "type", "listen");
        cJSON_AddStringToObject(root, "state", "stop");
        char *data = cJSON_PrintUnformatted(root);
        int len = strlen(data);
        esp_websocket_client_send_text(protocol_websocket->client, data, len, portMAX_DELAY);
        cJSON_free(data);
        MY_LOGI("停止监听包发送完成");
    }
    else
    {
        MY_LOGE("客户端未连接,停止监听包未发送");
    }
}

void Protocol_Websocket_Send_Abort(ProtocolWebsocket_t *protocol_websocket)
{
    if (Protocol_Websocket_Is_Connect(protocol_websocket))
    {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "session_id", protocol_websocket->sessionid);
        cJSON_AddStringToObject(root, "type", "abort");
        cJSON_AddStringToObject(root, "reason", "wake_word_detected");
        char *data = cJSON_PrintUnformatted(root);
        int len = strlen(data);
        esp_websocket_client_send_text(protocol_websocket->client, data, len, portMAX_DELAY);
        cJSON_free(data);
        MY_LOGI("语音中断包发送完成");
    }
    else
    {
        MY_LOGE("客户端未连接,语音中断包未发送");
    }
}

void Protocol_Websocket_Send_Opus(ProtocolWebsocket_t *protocol_websocket, char *data, int len)
{
    esp_websocket_client_send_bin(protocol_websocket->client, data, len, 5000);
}

void Ptotocol_Websocket_Send_MCP(ProtocolWebsocket_t *protocol_websocket, cJSON *payload)
{
    char *data = NULL;
    size_t len = 0;
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        MY_LOGE("MCProot创建失败,响应未发送");
        return;
    }
    
    char *session_id = Protocol_Websocket_Get_Session(protocol_websocket, &len);
    cJSON_AddStringToObject(root, "session_id", session_id);
    cJSON_AddStringToObject(root, "type", "mcp");
    if (cJSON_AddItemToObject(root, "payload", payload) == false)
    {
        cJSON_Delete(payload);
        cJSON_Delete(root);
        MY_LOGE("payload附着失败,响应未发送");
        return;
    }

    data = cJSON_PrintUnformatted(root);
    if(data == NULL){
        cJSON_Delete(root);
        MY_LOGE("MCPjson字符串生成失败,响应未发送");
        return;
    }
    if (Protocol_Websocket_Is_Connect(protocol_websocket))
    {
        esp_websocket_client_send_text(protocol_websocket->client, data, strlen(data), portMAX_DELAY);
        MY_LOGI("MCP响应发送完成:%s+++",data);
    }
    else
    {
        MY_LOGE("客户端未连接,响应未发送");
    }
    cJSON_free(data);
    cJSON_Delete(root);
}