#include "Bsp_NVS.h"
static void Bsp_NVS_UUID_Generate(char *uuid) {
    // 生成16字节随机数据（ESP32-S3硬件RNG）
    uint32_t rand_words[4];
    rand_words[0] = esp_random();
    rand_words[1] = esp_random();
    rand_words[2] = esp_random();
    rand_words[3] = esp_random();
    
    uint8_t *rand_bytes = (uint8_t *)rand_words;
    
    // 设置版本位 (第6字节高4位设为0100)
    rand_bytes[6] = (rand_bytes[6] & 0x0F) | 0x40;
    
    // 设置变体位 (第8字节高2位设为10)
    rand_bytes[8] = (rand_bytes[8] & 0x3F) | 0x80;
    
    // 格式化为标准UUID字符串
    snprintf(uuid, 37, 
            "%02x%02x%02x%02x-"
            "%02x%02x-"
            "%02x%02x-"
            "%02x%02x-"
            "%02x%02x%02x%02x%02x%02x",
            rand_bytes[0], rand_bytes[1], rand_bytes[2], rand_bytes[3],
            rand_bytes[4], rand_bytes[5], rand_bytes[6], rand_bytes[7],
            rand_bytes[8], rand_bytes[9], rand_bytes[10], rand_bytes[11],
            rand_bytes[12], rand_bytes[13], rand_bytes[14], rand_bytes[15]);
}

struct Bsp_NVS
{
    nvs_handle_t nvs_handle;
    char *uuid;
    char *mac;
};

Bsp_NVS_t *Bsp_NVS_Init(void)
{
    Bsp_NVS_t *bsp_nvs = malloc(sizeof(Bsp_NVS_t));
    nvs_flash_init();
    nvs_open("HWM", NVS_READWRITE, &(bsp_nvs->nvs_handle));
    bsp_nvs->uuid = (char *)malloc(UUID_SIZE);
    bsp_nvs->mac = (char *)malloc(MAC_SIZE);
    return bsp_nvs;
}

void Bsp_NVS_GetMess(Bsp_NVS_t *bsp_nvs)
{
    // 获取mac
    size_t maclen = MAC_SIZE;
    if (nvs_get_str(bsp_nvs->nvs_handle, "mac", bsp_nvs->mac, &maclen) == ESP_OK && maclen == MAC_SIZE)
    {
        MY_LOGI("nvs成功获取MAC: %s", bsp_nvs->mac);
    }
    else
    {
        uint8_t eth_mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
        snprintf(bsp_nvs->mac, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
                 eth_mac[0], eth_mac[1], eth_mac[2], eth_mac[3], eth_mac[4], eth_mac[5]);
        nvs_set_str(bsp_nvs->nvs_handle,"mac",bsp_nvs->mac);
        MY_LOGI("成功获取MAC并设置nvs: %s", bsp_nvs->mac);
    }
    // 获取uuid
    size_t uuidlen = UUID_SIZE;
    if (nvs_get_str(bsp_nvs->nvs_handle,"uuid",bsp_nvs->uuid,&uuidlen)==ESP_OK&&uuidlen==UUID_SIZE){
        MY_LOGI("nvs成功获取UUID: %s", bsp_nvs->uuid);
    }
    else{
        Bsp_NVS_UUID_Generate(bsp_nvs->uuid);
        nvs_set_str(bsp_nvs->nvs_handle,"uuid",bsp_nvs->uuid);
         MY_LOGI("成功获取UUID并设置nvs: %s", bsp_nvs->uuid);
    }
    nvs_commit(bsp_nvs->nvs_handle);
}

void Bsp_NVS_Deinit(Bsp_NVS_t *bsp_nvs){
    nvs_close(bsp_nvs->nvs_handle);
    free(bsp_nvs->uuid);
    free(bsp_nvs->mac);
    free(bsp_nvs);
}

void Bsp_NVS_ReadMess(Bsp_NVS_t *bsp_nvs,const char **mac,const char **uuid){
    *mac = bsp_nvs->mac;
    *uuid = bsp_nvs->uuid;
}
