#include "Audio_Decode.h"

struct Audio_Decode
{
    esp_audio_dec_handle_t decoder;
    RingbufHandle_t input_buffer;  // 输入缓冲区
    RingbufHandle_t output_buffer; // 输出缓冲区
    bool task_state;               // 任务运行状态
};

static void Decode_Task(void *args);

/// @brief 初始化解码器句柄,需手动释放
/// @param
/// @return
Audio_Decode_t *Audio_Decode_Init(void)
{
    // 申请句柄空间
    Audio_Decode_t *audio_decode = (Audio_Decode_t *)malloc(sizeof(Audio_Decode_t));
    // 任务--关
    audio_decode->task_state = false;
    // 配置解码器
    // int heap_size = esp_get_free_heap_size(); // 干嘛
    //.1 注册opus格式
    esp_opus_dec_register();
    //.2 设置opus配置
    esp_opus_dec_cfg_t opus_cfg = {
        .channel = ESP_AUDIO_MONO,
        .sample_rate = ESP_AUDIO_SAMPLE_RATE_16K,
        .frame_duration = ESP_OPUS_DEC_FRAME_DURATION_60_MS,
        .self_delimited = false,
    };
    //.3 构造通用接口配置
    esp_audio_dec_cfg_t dec_cfg = {
        .type = ESP_AUDIO_TYPE_OPUS,
        .cfg = &opus_cfg,
        .cfg_sz = sizeof(opus_cfg),
    };
    //.4 构造通用解码器接口句柄
    audio_decode->decoder = NULL;
    if (esp_audio_dec_open(&dec_cfg, &(audio_decode->decoder)) == ESP_AUDIO_ERR_OK)
    {
        MY_LOGI("解码器构建成功");
    }
    return audio_decode;
}

/// @brief 启动解码器
/// @param audio_decode
void Audio_Decode_Start(Audio_Decode_t *audio_decode)
{

    if (audio_decode->task_state)
    {
        return;
    }
    // 启动编码器任务
    audio_decode->task_state = true;
    xTaskCreatePinnedToCoreWithCaps(Decode_Task, "decode", 32 * 1024, audio_decode, 5, NULL, 0, MALLOC_CAP_SPIRAM);
}

/// @brief 设置解码器IObuffer
/// @param audio_decode
/// @param input_buffer
/// @param output_buffer
void Audio_Decode_Set_Buffer(Audio_Decode_t *audio_decode, RingbufHandle_t input_buffer, RingbufHandle_t output_buffer)
{
    audio_decode->input_buffer = input_buffer;
    audio_decode->output_buffer = output_buffer;
}

static void Decode_Task(void *args)
{
    // 获取句柄
    Audio_Decode_t *audio_decode = (Audio_Decode_t *)args;
    MY_LOGI("解码器任务,启动!");
    // 原始压缩输入块
    esp_audio_dec_in_raw_t raw;
    // 解压输出块
    esp_audio_dec_out_frame_t out_frame;
    out_frame.len = 1920;
    out_frame.buffer = malloc(out_frame.len);

    uint8_t *temp_buffer;
    size_t temp_len;

    int ret = 0;

    while (audio_decode->task_state)
    {
        // 读取输入buffer到temp_buffer,temp_len
        temp_buffer = xRingbufferReceive(audio_decode->input_buffer, &(temp_len), portMAX_DELAY);
        // 将temp参数赋给raw
        raw.buffer = temp_buffer;
        raw.len = temp_len;
        while (raw.len)
        {
            ret = esp_audio_dec_process(audio_decode->decoder, &raw, &out_frame);
            // 如果输出空间不够
            if (ret == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH)
            {
                uint8_t *new_buf = realloc(out_frame.buffer, out_frame.needed_size);
                out_frame.buffer = new_buf;
                out_frame.len = out_frame.needed_size;
                continue;
            }
            // 如果解码失败
            if (ret != ESP_AUDIO_ERR_OK)
            {
                // 跳过当前帧
                break;
            }
            // 将out_frame放入输出缓冲区
            //xRingbufferSend(audio_decode->output_buffer, out_frame.buffer, out_frame.decoded_size, portMAX_DELAY);
            Bsp_ES8311_Write_To_Player(out_frame.buffer, out_frame.decoded_size);
            raw.len -= raw.consumed;
            raw.buffer += raw.consumed;
        }
        // 归还输入缓冲区资源
        vRingbufferReturnItem(audio_decode->input_buffer, temp_buffer);
        vTaskDelay(5);
    }

    // 释放资源
    free(out_frame.buffer);
    // 删除任务
    vTaskDelete(NULL);
}
