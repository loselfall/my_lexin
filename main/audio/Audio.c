#include "Audio.h"

struct Audio
{
    Audio_SR_t *audio_sr;
    Audio_Encode_t *audio_encode;
    Audio_Decode_t *audio_decode;
    RingbufHandle_t encode_output_buffer;
    RingbufHandle_t decode_input_buffer;
};

struct Opus_Frame
{
    uint8_t *data;
    size_t len;
    size_t size;
};

static void Test_Task(void *args);

Audio_t *Audio_Init(void)
{
    Audio_t *audio = (Audio_t *)malloc(sizeof(Audio_t));
    audio->audio_sr = Audio_SR_Init();
    audio->audio_encode = Audio_Encode_Init();
    audio->audio_decode = Audio_Decode_Init();
    RingbufHandle_t encode_input_buffer = xRingbufferCreateWithCaps(32 * 1024, RINGBUF_TYPE_BYTEBUF, MALLOC_CAP_SPIRAM);
    audio->encode_output_buffer = xRingbufferCreateWithCaps(32 * 1024, RINGBUF_TYPE_NOSPLIT, MALLOC_CAP_SPIRAM);
    audio->decode_input_buffer = xRingbufferCreateWithCaps(32 * 1024, RINGBUF_TYPE_NOSPLIT, MALLOC_CAP_SPIRAM);
    RingbufHandle_t decode_output_buffer = xRingbufferCreateWithCaps(32 * 1024, RINGBUF_TYPE_NOSPLIT, MALLOC_CAP_SPIRAM);
    Audio_SR_Set_Fetch_Out_Buffer(audio->audio_sr, encode_input_buffer);
    Audio_Encode_Set_Buffer(audio->audio_encode, encode_input_buffer, audio->encode_output_buffer);
    Audio_Decode_Set_Buffer(audio->audio_decode, audio->decode_input_buffer, decode_output_buffer);
    return audio;
}

void Audio_Start(Audio_t *audio)
{
    Audio_Decode_Start(audio->audio_decode);
    Audio_Encode_Start(audio->audio_encode);
    Audio_SR_Start(audio->audio_sr);
}

/// @brief 从编码器读数据
/// @param audio 
/// @param data 数据空间
/// @param len 接收的实际长度
/// @param size 数据空间的容量
/// @return 
esp_err_t Audio_Read(Audio_t *audio, uint8_t *data, size_t *len, size_t size)
{
    uint8_t *tempdata = xRingbufferReceive(audio->encode_output_buffer, len, portMAX_DELAY);
    if (size < *len)
    {
        printf("帧数据空间不足,需要%u\r\n",*len);
        vRingbufferReturnItem(audio->encode_output_buffer, tempdata);
        return ESP_FAIL;
    }

    memcpy(data, tempdata, *len);
    vRingbufferReturnItem(audio->encode_output_buffer, tempdata);
    return ESP_OK;
}

/// @brief 向解码器写数据
/// @param audio 
/// @param data 
/// @param len 
void Audio_Write(Audio_t *audio, uint8_t *data, size_t len)
{
    xRingbufferSend(audio->decode_input_buffer, data, len, portMAX_DELAY);
}

void Audio_Callback_Register(Audio_t *audio, Callback_Vad_t CBvad, Callback_Wakeup_t CBwakeup)
{
    Audio_SR_State_Callback_Register(audio->audio_sr, CBvad, CBwakeup);
}

void Audio_Fall_Sleep(Audio_t *audio){
    Audio_SR_Fall_Sleep(audio->audio_sr);
}

void Audio_test(Audio_t *audio)
{
    xTaskCreatePinnedToCoreWithCaps(Test_Task, "test", 32 * 1024, audio, 5, NULL, 0, MALLOC_CAP_SPIRAM);
}


/// @brief 回环测试
/// @param args
static void Test_Task(void *args)
{
    Audio_t *audio = (Audio_t *)args;
    struct Opus_Frame frame = {
        .data = NULL,
        .len = 0,
        .size = 2 * 1024,
    };
    frame.data = (uint8_t *)malloc(frame.size);
    while (1)
    {
        if (Audio_Read(audio, frame.data, &(frame.len), frame.size) == ESP_OK)
        {
            Audio_Write(audio, frame.data, frame.len);
        }
        vTaskDelay(10);
    }
    free(frame.data);
    vTaskDelete(NULL);
}


