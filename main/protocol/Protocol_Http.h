#ifndef __PROTOCOL_HTTP_H__
#define __PROTOCOL_HTTP_H__

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_app_format.h"
#include "esp_app_desc.h"
#include <esp_wifi.h>

#include "esp_tls.h"

#include "esp_crt_bundle.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"

#include "Com_Debug.h"
#include "cJSON.h"


#define XIAOZHI_OTA_URL "https://api.tenclass.net/xiaozhi/ota/"

typedef struct Protocal_Http Protocal_Http_t;

Protocal_Http_t* Protocal_Http_Init(const char *mac, const char *uuid);



#endif /* __PROTOCOL_HTTP_H__ */
