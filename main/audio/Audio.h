#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Audio_SR.h"
#include "Audio_Decode.h"
#include "Audio_Encode.h"

typedef struct Audio Audio_t;

Audio_t *Audio_Init(void);

void Audio_Start(Audio_t *audio);

esp_err_t Audio_Read(Audio_t *audio, uint8_t *data,size_t *len,size_t size);

void Audio_Write(Audio_t *audio,uint8_t *data, size_t len);

void Audio_Callback_Register(Audio_t *audio, Callback_Vad_t CBvad, Callback_Wakeup_t CBwakeup);

void Audio_Fall_Sleep(Audio_t *audio);

void Audio_test(Audio_t *audio);
#endif /* __AUDIO_H__ */
