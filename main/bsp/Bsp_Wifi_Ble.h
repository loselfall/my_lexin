#ifndef __BSP_WIFI_BLE_H__
#define __BSP_WIFI_BLE_H__

#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>

#include <wifi_provisioning/manager.h>

#define CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
#define CONFIG_EXAMPLE_PROV_SHOW_QR

#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
#include <wifi_provisioning/scheme_ble.h>
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */

#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP
#include <wifi_provisioning/scheme_softap.h>
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */
#include "qrcode.h"

void Bsp_Wifi_Ble_Init(void);

#endif /* __Bsp_WIFI_BLE_H__ */
