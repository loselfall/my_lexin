#ifndef __BSP_ES8311_H__
#define __BSP_ES8311_H__

#include "esp_idf_version.h"

#include "driver/i2s_std.h"
#include "driver/i2s_tdm.h"
#include "soc/soc_caps.h"

#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "unity.h"

#include "driver/i2c.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2S_MAX_KEEP SOC_I2S_NUM
#define I2C_SDA_PIN (0)
#define I2C_SCL_PIN (1)
#define I2S_BCK_PIN (2)
#define I2S_MCK_PIN (3)
#define I2S_DATA_IN_PIN (4)
#define I2S_DATA_WS_PIN (5)
#define I2S_DATA_OUT_PIN (6)
#define PA_EN_PIN (7)

typedef struct
{
    i2s_chan_handle_t tx_handle;
    i2s_chan_handle_t rx_handle;
} i2s_keep_t;

void Bsp_ES8311_Init(void);

void test(void);

void Bsp_ES8311_Read_From_Micro(void *data, int len);


void Bsp_ES8311_Write_To_Player(void *data, int len);

void Bsp_ES8311_Set_Volume(int volume);
#endif /* __Bsp_ES8311_H__ */
