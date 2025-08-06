#include "Audio_Encode.h"

static void Encode_Task(void *args);

/// @brief 编码器句柄
struct Audio_Encode
{
    esp_audio_enc_handle_t encoder; // 通用编码器接口句柄
    RingbufHandle_t input_buffer;   // 输入缓冲区
    RingbufHandle_t output_buffer;  // 输出缓冲区
    bool task_state;                // 任务运行状态
};

/// @brief 编码器初始化
/// @param
/// @return
Audio_Encode_t *Audio_Encode_Init(void)
{
    // 为编码器申请空间
    Audio_Encode_t *audio_encode = (Audio_Encode_t *)malloc(sizeof(Audio_Encode_t));
    // 默认任务状态--关
    audio_encode->task_state = false;
    // 配置编码器
    // int heap_size = esp_get_free_heap_size(); // 在干嘛？
    //.1 注册opus格式
    esp_opus_enc_register();
    //.2 设置opus配置
    esp_opus_enc_config_t opus_cfg = {
        .sample_rate = ESP_AUDIO_SAMPLE_RATE_16K,
        .channel = ESP_AUDIO_MONO,
        .bits_per_sample = ESP_AUDIO_BIT16,
        .bitrate = 90000,
        .frame_duration = ESP_OPUS_ENC_FRAME_DURATION_60_MS,
        .application_mode = ESP_OPUS_ENC_APPLICATION_VOIP,
        .complexity = 0,
        .enable_fec = false,
        .enable_dtx = false,
        .enable_vbr = false,
    };
    //.3 构造通用接口配置
    esp_audio_enc_config_t enc_cfg = {
        .type = ESP_AUDIO_TYPE_OPUS,
        .cfg = &opus_cfg,
        .cfg_sz = sizeof(opus_cfg)};
    //.4 构造通用编码器接口句柄
    audio_encode->encoder = NULL;
    if (esp_audio_enc_open(&enc_cfg, &(audio_encode->encoder)) == ESP_AUDIO_ERR_OK)
    {
        MY_LOGI("编码器构建成功");
    }
    return audio_encode;
}

/// @brief 编码器启动
/// @param audio_encode
void Audio_Encode_Start(Audio_Encode_t *audio_encode)
{
    if (audio_encode->task_state)
    {
        return;
    }
    // 启动编码器任务
    audio_encode->task_state = true;
    xTaskCreatePinnedToCoreWithCaps(Encode_Task, "encode", 32 * 1024, audio_encode, 5, NULL, 0, MALLOC_CAP_SPIRAM);
}

/// @brief 设置编码IO缓冲区
/// @param audio_encode 编码器
/// @param input_buffer 输入缓冲
/// @param output_buffer 输出缓冲
void Audio_Encode_Set_Buffer(Audio_Encode_t *audio_encode, RingbufHandle_t input_buffer, RingbufHandle_t output_buffer)
{
    audio_encode->input_buffer = input_buffer;
    audio_encode->output_buffer = output_buffer;
}

/// @brief 从input_buffer中读取固定长度的数据到in_frame中
/// @param audio_encode
/// @param in_frame in_frame->len的值为in_frame->buffer的容量
static void Audio_Encode_Set_Frame_Data(Audio_Encode_t *audio_encode, esp_audio_enc_in_frame_t *in_frame)
{
    size_t Remainlen = in_frame->len;
    uint8_t *Remainbuffer = in_frame->buffer;
    size_t templen = 0;
    uint8_t *tempbuffer = NULL;
    while (Remainlen)
    {
        tempbuffer = xRingbufferReceiveUpTo(audio_encode->input_buffer, &templen, portMAX_DELAY, Remainlen);
        if (templen > 0)
        {
            memcpy(Remainbuffer, tempbuffer, templen);
            vRingbufferReturnItem(audio_encode->input_buffer, tempbuffer);
            Remainbuffer += templen;
            Remainlen -= templen;
        }
    }
}

/// @brief 不断从input_buffer读取数据并编码,结果存入output_buffer
/// @param args audio_encode
static void Encode_Task(void *args)
{

    MY_LOGI("编码任务,启动!");
    // 获取编码器
    Audio_Encode_t *audio_encode = args;
    // 创建帧容器
    esp_audio_enc_in_frame_t *in_frame = (esp_audio_enc_in_frame_t *)malloc(sizeof(esp_audio_enc_in_frame_t));
    esp_audio_enc_out_frame_t *out_frame = (esp_audio_enc_out_frame_t *)malloc(sizeof(esp_audio_enc_out_frame_t));

    // 计算帧长
    int in_len, out_len;
    esp_audio_enc_get_frame_size(audio_encode->encoder, &in_len, &out_len);
    in_frame->len = in_len;
    out_frame->len = out_len;
    // 申请帧数据空间
    in_frame->buffer = (uint8_t *)malloc(in_frame->len);
    out_frame->buffer = (uint8_t *)malloc(out_frame->len);
    // 编码
    while (audio_encode->task_state)
    {
        Audio_Encode_Set_Frame_Data(audio_encode, in_frame);
        esp_audio_enc_process(audio_encode->encoder, in_frame, out_frame);
        xRingbufferSend(audio_encode->output_buffer, out_frame->buffer, out_frame->encoded_bytes, portMAX_DELAY);
        vTaskDelay(10);
    }
    MY_LOGE("编码任务,结束!");
    // 释放资源
    free(in_frame->buffer);
    free(out_frame->buffer);
    free(in_frame);
    free(out_frame);
    // 结束任务
    vTaskDelete(NULL);
}
