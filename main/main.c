#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Com_Debug.h"

#include "Bsp_Wifi_Ble.h"
#include "Bsp_ES8311.h"
#include "Bsp_WS2812.h"
#include "Bsp_NVS.h"

#include "Audio_SR.h"
#include "Audio_Encode.h"
#include "Audio_Decode.h"
#include "Audio.h"

#include "Protocol_Http.h"
#include "Protocol_Websocket.h"
#include "Com_State.h"

Audio_t *audio;
ProtocolWebsocket_t *protocol_websocket;
Bsp_NVS_t *bsp_nvs;

void App_MCP_Handler(int id_num, char *method_str, cJSON *params_json);

void Vad_Callback(void *args, vad_state_t state);
void Wakeup_Callback(void *args);
void Json_Callback(ProtocolWebsocket_t *protocol_websocket, const char *data, int len);
void Opus_Callback(ProtocolWebsocket_t *protocol_websocket, const char *data, int len);
void Fin_Callback(void);

void app_main(void)
{
    const char *mac = NULL, *uuid = NULL, *url = "wss://api.tenclass.net/xiaozhi/v1/";
    // Bsp初始化
    Bsp_ES8311_Init();
    Bsp_WS2812_Init();
    Bsp_Wifi_Ble_Init();

    bsp_nvs = Bsp_NVS_Init();
    Bsp_NVS_GetMess(bsp_nvs);
    Bsp_NVS_ReadMess(bsp_nvs, &mac, &uuid);

    audio = Audio_Init();
    Audio_Callback_Register(audio, Vad_Callback, Wakeup_Callback);
    Audio_Start(audio);

    Protocal_Http_Init(mac, uuid);
    protocol_websocket = Protocol_Websocket_Init(url, mac, uuid);
    Protocol_Websocket_CB_register(protocol_websocket, Json_Callback, Opus_Callback, Fin_Callback);

    Bsp_WS2812_Set(RED);
    uint8_t *opusdata = malloc(1024);
    size_t opuslen = 0;
    while (1)
    {
        if (Protocol_Websocket_Is_Connect(protocol_websocket))
        {
            Audio_Read(audio, opusdata, &opuslen, 1024);
            Protocol_Websocket_Send_Opus(protocol_websocket, (char *)opusdata, opuslen);
        }

        vTaskDelay(10);
    }
}

void Vad_Callback(void *args, vad_state_t state)
{
    if (com_state != SPEAK && com_state != UNCONNECT)
    {
        if (state == VAD_SPEECH)
        {

            Com_State_Change(LISTEN);
            // Protocol_Websocket_Send_Listen_Start(protocol_websocket);
        }
        else
        {

            Com_State_Change(IDLE);
        }
        MY_LOGI("%s", state == VAD_SPEECH ? "SPEECH" : "SILENCE");
    }
}

void open_websocket_listen_week(void)
{
    printf("**ListenUp**\r\n");
    Protocol_Websocket_Send_Listen_Start(protocol_websocket);
}
void stop_websocket_listen_week(void)
{
    printf("**ListenDown**\r\n");
    Protocol_Websocket_Send_Listen_Stop(protocol_websocket);
}

void Wakeup_Callback(void *args)
{
    if (com_state == UNCONNECT)
    {
        Protocol_Websocket_Start(protocol_websocket);
        Protocol_Websocket_Send_Hello(protocol_websocket);
        open_websocket_listen_week();
    }
    else if (com_state != LISTEN)
    {
        Protocol_Websocket_Send_Abort(protocol_websocket);
    }

    Protocol_Websocket_Send_Wakeup_Word(protocol_websocket);
    // vTaskDelay(1000);
    // open_websocket_listen_week();
    Com_State_Change(IDLE);
}

void Json_Callback(ProtocolWebsocket_t *protocol_websocket, const char *data, int len)
{
    printf("%.*s\r\n", len, data);
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root == NULL)
    {
        MY_LOGE("json解析失败");
        return;
    }
    cJSON *type_json = cJSON_GetObjectItem(root, "type");
    if (type_json == NULL)
    {
        MY_LOGE("未找到type");
    }
    else
    {
        char *type_str = cJSON_GetStringValue(type_json);
        if (strcmp(type_str, "hello") == 0)
        {

            cJSON *session_json = cJSON_GetObjectItem(root, "session_id");
            if (session_json == NULL)
            {
                MY_LOGE("session_json 解析异常");
                cJSON_Delete(root);
                return;
            }

            char *session_str = cJSON_GetStringValue(session_json);
            size_t str_len = strlen(session_str);
            if (session_str == NULL || str_len == 0)
            {
                MY_LOGE("session ID 异常");
                cJSON_Delete(root);
                return;
            }

            size_t id_len;
            char *session_id = Protocol_Websocket_Get_Session(protocol_websocket, &id_len);

            if (session_id == NULL)
            {
                session_id = malloc(str_len + 1);
                memcpy(session_id, session_str, str_len + 1);
                Protocol_Websocket_Set_Session(protocol_websocket, session_id, str_len);
                MY_LOGI("session ID[%u]{%s}初始化", str_len, session_id);
            }
            else if (strcmp(session_id, session_str) != 0)
            {
                if (str_len != id_len)
                {
                    session_id = realloc(session_id, str_len + 1);
                }
                memcpy(session_id, session_str, str_len + 1);
                Protocol_Websocket_Set_Session(protocol_websocket, session_id, str_len);
                MY_LOGI("session ID[%u]{%s}更新", str_len, session_id);
            }
        }
        else if (strcmp(type_str, "tts") == 0)
        {
            cJSON *state_json = cJSON_GetObjectItem(root, "state");
            if (type_json == NULL)
            {
                MY_LOGE("未找到state");
            }
            else
            {
                char *state_str = cJSON_GetStringValue(state_json);
                if (strcmp(state_str, "start") == 0)
                {
                    Com_State_Change(SPEAK);
                }
                else if (strcmp(state_str, "stop") == 0)
                {
                    Com_State_Change(IDLE);
                }
            }
        }
        else if (strcmp(type_str, "stt") == 0)
        {
        }
        else if (strcmp(type_str, "llm") == 0)
        {
        }
        else if (strcmp(type_str, "mcp") == 0)
        {
            // 提取payload
            cJSON *payload_json = cJSON_GetObjectItemCaseSensitive(root, "payload");
            if (payload_json == NULL)
            {
                printf("payload_json获取失败\r\n");
            }
            else
            {
                cJSON *id_json = cJSON_GetObjectItem(payload_json, "id");
                cJSON *method_json = cJSON_GetObjectItem(payload_json, "method");
                cJSON *params_json = cJSON_GetObjectItem(payload_json, "params");
                if (id_json == NULL || method_json == NULL || params_json == NULL)
                {
                    printf("payload子项不全\r\n");
                }
                else
                {
                    int id_num = (int)cJSON_GetNumberValue(id_json);
                    char *method_str = cJSON_GetStringValue(method_json);
                    App_MCP_Handler(id_num, method_str, params_json);
                }
            }
        }
    }
    cJSON_Delete(root);
}

void Opus_Callback(ProtocolWebsocket_t *protocol_websocket, const char *data, int len)
{
    Audio_Write(audio, (uint8_t *)data, len);
}

void Fin_Callback(void)
{
    Com_State_Change(UNCONNECT);
    Audio_Fall_Sleep(audio);
    MY_LOGE("断开连接,退出唤醒");
}

/// @brief 处理MCP协议消息
/// @param id_num 请求ID
/// @param method_str 方法如initial、tool\call
/// @param params_json 参数
void App_MCP_Handler(int id_num, char *method_str, cJSON *params_json)
{

    cJSON *payload = cJSON_CreateObject();
    if (payload == NULL)
    {
        MY_LOGE("payload创建失败");
        return;
    }
    cJSON_AddStringToObject(payload, "jsonrpc", "2.0");
    cJSON_AddNumberToObject(payload, "id", id_num);
    cJSON *result = cJSON_AddObjectToObject(payload, "result");
    if (result == NULL)
    {
        MY_LOGE("result创建失败");
        cJSON_Delete(payload);
        return;
    }
    if (strcmp(method_str, "initialize") == 0)
    { // 处理初始化
        /*
            {
                "jsonrpc": "2.0",
                "id": ?,
                "result": {
                    "protocolVersion": "2024-11-05",
                    "capabilities": {
                        "tools": {}
                    },
                    "serverInfo": {
                    "name": "wangwang",
                    "version": "1.8.2"
                    }
                }
            }
        */
        MY_LOGW("收到MCP初始化请求");
        cJSON_AddStringToObject(result, "protocolVersion", "2024-11-05");
        cJSON *capabilities = cJSON_AddObjectToObject(result, "capabilities");
        cJSON *serverInfo = cJSON_AddObjectToObject(result, "serverInfo");
        if (capabilities == NULL || serverInfo == NULL)
        {
            MY_LOGE("capabilities/serverInfo创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddStringToObject(serverInfo, "name", "wangwang");
        cJSON_AddStringToObject(serverInfo, "version", "1.8.2");
        cJSON *tools = cJSON_AddObjectToObject(capabilities, "tools");
        if (tools == NULL)
        {
            MY_LOGE("tools创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddBoolToObject(tools, "list", true);
        cJSON_AddBoolToObject(tools, "call", true);
    }
    else if (strcmp(method_str, "tools/list") == 0)
    {
        MY_LOGW("收到工具清单访问");
        // 提供工具列表
        cJSON *tools = cJSON_AddArrayToObject(result, "tools");
        if (tools == NULL)
        {
            MY_LOGE("tools数组创建失败");
            cJSON_Delete(payload);
            return;
        }

        // 设备1
        cJSON *tool_setvol = cJSON_CreateObject();
        if (tool_setvol == NULL)
        {
            MY_LOGE("tool_setvol数组创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddItemToArray(tools, tool_setvol);

        cJSON_AddStringToObject(tool_setvol, "name", "set_volume");
        cJSON_AddStringToObject(tool_setvol, "description", "Set the volume of the audio speaker,volume is 50,initially.");
        cJSON *ischema_val = cJSON_AddObjectToObject(tool_setvol, "inputSchema");
        if (ischema_val == NULL)
        {
            MY_LOGE("ischema_val创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddStringToObject(ischema_val, "type", "object");
        const char *items_val[1] = {"volume"};
        cJSON *required = cJSON_CreateStringArray(items_val, 1);
        if (required == NULL)
        {
            MY_LOGE("ischema_val创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddItemToObject(ischema_val, "required", required);
        cJSON *prop_val = cJSON_AddObjectToObject(ischema_val, "properties");
        if (prop_val == NULL)
        {
            MY_LOGE("ischema_val创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON *volume = cJSON_AddObjectToObject(prop_val, "volume");
        if (volume == NULL)
        {
            MY_LOGE("ischema_val创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddStringToObject(volume, "type", "integer");
        cJSON_AddNumberToObject(volume, "minimum", 0);
        cJSON_AddNumberToObject(volume, "maximum", 100);

        // 设备2
        cJSON *tool_setlight = cJSON_CreateObject();
        if (tool_setlight == NULL)
        {
            MY_LOGE("tool_setlight创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddItemToArray(tools, tool_setlight);

        cJSON_AddStringToObject(tool_setlight, "name", "light_switch");
        cJSON_AddStringToObject(tool_setlight, "description", "Control the light state (on/off)");
        cJSON *ischema_light = cJSON_AddObjectToObject(tool_setlight, "inputSchema");
        if (ischema_light == NULL)
        {
            MY_LOGE("ischema_val创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddStringToObject(ischema_light, "type", "object");
        const char *items_light[1] = {"state"};
        cJSON *required2 = cJSON_CreateStringArray(items_light, 1);
        if (required2 == NULL)
        {
            MY_LOGE("required2数组创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddItemToObject(ischema_light, "required", required2);
        cJSON *prop2_val = cJSON_AddObjectToObject(ischema_light, "properties");
        if (prop2_val == NULL)
        {
            MY_LOGE("ischema_val创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON *state = cJSON_AddObjectToObject(prop2_val, "state");
        if (state == NULL)
        {
            MY_LOGE("ischema_val创建失败");
            cJSON_Delete(payload);
            return;
        }
        cJSON_AddStringToObject(state, "type", "boolean");

        // char *tool_list_str = cJSON_PrintUnformatted(payload);
        // if (tool_list_str != NULL)
        // {
        //     MY_LOGE("%s", tool_list_str);
        //     cJSON_free(tool_list_str);
        // }
        // cJSON_Delete(payload);
        // return ;
    }
    else if (strcmp(method_str, "tools/call") == 0)
    { // 响应功能1、2、3
        MY_LOGW("收到设备控制访问");
        bool is_ok = false;
        cJSON *name_js = cJSON_GetObjectItem(params_json, "name");
        cJSON *argu_js = cJSON_GetObjectItem(params_json, "arguments");
        if (name_js == NULL || argu_js == NULL)
        {
            cJSON_Delete(payload);
            return;
        }
        char *name_str = cJSON_GetStringValue(name_js);
        if (strcmp(name_str, "set_volume") == 0)
        {
            // 音量设备控制
            cJSON *volume_js = cJSON_GetObjectItem(argu_js, "volume");
            if (volume_js == NULL)
            {
                cJSON_Delete(payload);
                return;
            }
            int vol = cJSON_GetNumberValue(volume_js);
            Bsp_ES8311_Set_Volume(vol);
            is_ok = true;
        }
        else if (strcmp(name_str, "light_switch") == 0)
        {
            // 灯光设备控制
            cJSON *light_j = cJSON_GetObjectItem(argu_js, "state");
            if (light_j == NULL)
            {
                cJSON_Delete(payload);
                return;
            }
            bool sw = cJSON_IsTrue(light_j);
            if (sw)
            {
                Bsp_WS2812_Open();
                Bsp_WS2812_Set(BLUE);
            }
            else
            {
                Bsp_WS2812_Close();
            }
            is_ok = true;
        }
        if (is_ok)
        {
            // 生成content数组
            cJSON *conarr_js = cJSON_CreateArray();
            cJSON *con_1_js = cJSON_CreateObject();
            if (conarr_js == NULL || con_1_js == NULL)
            {
                cJSON_Delete(payload);
                return;
            }
            cJSON_AddStringToObject(con_1_js, "type", "text");
            cJSON_AddStringToObject(con_1_js, "text", "true");
            cJSON_AddItemToArray(conarr_js, con_1_js);
            cJSON_AddItemToObject(result, "content", conarr_js);
            cJSON_AddFalseToObject(result, "isError");
        }
    }
    else
    {
        cJSON_Delete(payload);
        return;
    }
    Ptotocol_Websocket_Send_MCP(protocol_websocket, payload);
}