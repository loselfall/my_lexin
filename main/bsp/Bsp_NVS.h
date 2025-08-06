#ifndef __BSP_NVS_H__
#define __BSP_NVS_H__

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "Com_Debug.h"
#include "esp_random.h"

#define UUID_SIZE 37//带\0
#define MAC_SIZE 18//带\0

typedef struct Bsp_NVS Bsp_NVS_t;


Bsp_NVS_t* Bsp_NVS_Init(void);

void Bsp_NVS_GetMess(Bsp_NVS_t* bsp_nvs);

void Bsp_NVS_Deinit(Bsp_NVS_t *bsp_nvs);

void Bsp_NVS_ReadMess(Bsp_NVS_t *bsp_nvs,const char **mac,const char **uuid);

#endif /* __BSP_NVS_H__ */

