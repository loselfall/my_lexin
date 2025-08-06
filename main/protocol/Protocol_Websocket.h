#ifndef __PROTOCOL_WEBSOCKET_H__
#define __PROTOCOL_WEBSOCKET_H__

#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_crt_bundle.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_websocket_client.h"

#include "esp_event.h"
#include <cJSON.h>

#include "Com_Debug.h"
#include "Com_State.h"


typedef struct ProtocolWebsocket ProtocolWebsocket_t;
typedef void(*StrCallback)(ProtocolWebsocket_t *protocol_websocket,const char*,int);
ProtocolWebsocket_t *Protocol_Websocket_Init(const char *url,const char *mac, const char *uuid);
void Protocol_Websocket_Start(ProtocolWebsocket_t * protocol_websocket);

void Protocol_Websocket_Close(ProtocolWebsocket_t * protocol_websocket);

bool Protocol_Websocket_Is_Connect(ProtocolWebsocket_t *protocol_websocket);


void Protocol_Websocket_CB_register(ProtocolWebsocket_t *protocol_websocket, StrCallback jsoncb, StrCallback opuscb,void(*finishcb)(void));

void Protocol_Websocket_Set_Session(ProtocolWebsocket_t *protocol_websocket,char*session_id,size_t len);

char* Protocol_Websocket_Get_Session(ProtocolWebsocket_t *protocol_websocket,size_t *len);

void Protocol_Websocket_Send_Hello(ProtocolWebsocket_t *protocol_websocket);

void Protocol_Websocket_Send_Wakeup_Word(ProtocolWebsocket_t *protocol_websocket);

void Protocol_Websocket_Send_Listen_Start(ProtocolWebsocket_t *protocol_websocket);

void Protocol_Websocket_Send_Listen_Stop(ProtocolWebsocket_t *protocol_websocket);

void Protocol_Websocket_Send_Abort(ProtocolWebsocket_t *protocol_websocket);

void Protocol_Websocket_Send_Opus(ProtocolWebsocket_t *protocol_websocket,char*data,int len);

void Ptotocol_Websocket_Send_MCP(ProtocolWebsocket_t *protocol_websocket, cJSON *payload);
#endif /* __PROTOCOL_WEBSOCKET_H__ */
