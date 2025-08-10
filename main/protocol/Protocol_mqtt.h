#ifndef __PROTOCOL_MQTT_H__
#define __PROTOCOL_MQTT_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "cJSON.h"

#include "esp_log.h"
#include "mqtt_client.h"

void Protocol_mqtt_Init(void);
void Protocol_mqtt_Publish(bool sw,int speed);

#endif /* __PROTOCOL_MQTT_H__ */
