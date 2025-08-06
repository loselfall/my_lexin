#ifndef __AUDIO_ENCODE_H__
#define __AUDIO_ENCODE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"
#include "unity.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_audio_enc_default.h"
#include "esp_audio_enc_reg.h"
#include "esp_audio_enc.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"

#include "Com_Debug.h"

typedef struct Audio_Encode Audio_Encode_t;

Audio_Encode_t* Audio_Encode_Init(void);

void Audio_Encode_Start(Audio_Encode_t* audio_encode);

void Audio_Encode_Set_Buffer(Audio_Encode_t* audio_encode,RingbufHandle_t input_buffer,RingbufHandle_t output_buffer);

#endif /* __AUDIO_ENCODE_H__ */
