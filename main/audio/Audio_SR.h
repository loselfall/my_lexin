#ifndef __AUDIO_SR_H__
#define __AUDIO_SR_H__

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_process_sdkconfig.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_iface.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "model_path.h"
#include "freertos/ringbuf.h"

#include "Bsp_ES8311.h"
#include "Com_State.h"


typedef struct Audio_SR Audio_SR_t;
typedef void (*Callback_Vad_t)(void* args,vad_state_t state);
typedef void (*Callback_Wakeup_t)(void* args);

Audio_SR_t* Audio_SR_Init(void);

void Audio_SR_Start(Audio_SR_t* audio_sr);

void Audio_SR_State_Callback_Register(Audio_SR_t* audio_sr,Callback_Vad_t CBvad,Callback_Wakeup_t CBwakeup);

void Audio_SR_Set_Fetch_Out_Buffer(Audio_SR_t* audio_sr,RingbufHandle_t buffer);

void Audio_SR_Fall_Sleep(Audio_SR_t* audio_sr);
void fetch_error_deal(void);



#endif /* __AUDIO_SR_H__ */
