#ifndef __AUDIO_DECODE_H__
#define __AUDIO_DECODE_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "unity.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_audio_dec_default.h"
#include "esp_audio_dec.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"

//测试用
#include"Bsp_ES8311.h"

#include "Com_Debug.h"

typedef struct Audio_Decode Audio_Decode_t;

Audio_Decode_t* Audio_Decode_Init(void);

void Audio_Decode_Start(Audio_Decode_t* audio_decode);

void Audio_Decode_Set_Buffer(Audio_Decode_t* audio_decode,RingbufHandle_t input_buffer,RingbufHandle_t output_buffer);



#endif /* __AUDIO_DECODE_H__ */
