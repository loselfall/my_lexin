#include "Audio_SR.h"

static void fetch_Task(void *args);
static void feed_Task(void *args);

/// @brief 声音识别句柄
struct Audio_SR
{
    esp_afe_sr_iface_t *afe_handle; // 模拟前端句柄
    esp_afe_sr_data_t *afe_data;    // 模拟前端数据

    Callback_Vad_t CBvad; // 人声检测事件回调
    void *vadargs;
    Callback_Wakeup_t CBwakeup; // 唤醒词检测事件回调
    void *wakeupargs;

    vad_state_t last_vadstate; // 上一帧人声状态
    bool wake_flag;            // 唤醒标志
    bool task_flag;            // 任务运行标志
    TaskHandle_t fetch_handle;
    TaskHandle_t feed_handle;
    RingbufHandle_t fetch_out_buffer;
};

/// @brief 声音识别模块初始化
/// @param
/// @return
Audio_SR_t *Audio_SR_Init(void)
{
    // 申请句柄空间
    Audio_SR_t *audio_sr = (Audio_SR_t *)malloc(sizeof(Audio_SR_t));
    audio_sr->wake_flag = false;
    audio_sr->task_flag = false;
    audio_sr->last_vadstate = VAD_SILENCE;
    audio_sr->fetch_out_buffer = NULL;

    // 初始化模拟前端配置并创建实例
    //.1 初始化模型
    srmodel_list_t *models = esp_srmodel_init("model");
    //.2 初始化配置信息
    afe_config_t *afe_config = afe_config_init("M", models, AFE_TYPE_SR, AFE_MODE_HIGH_PERF);

    //.2.1 人声活动检测配置修改
    afe_config->vad_mode = VAD_MODE_3; // 默认0,改为3减少 误 检
    //.2.2 唤醒词检测
    afe_config->wakenet_mode = DET_MODE_95; // 默认0,改为1减少 漏 检
    //.2.3 打印初始化配置信息
    afe_config_print(afe_config);
    //.3 获取afe句柄
    audio_sr->afe_handle = esp_afe_handle_from_config(afe_config);
    //.4 创建实例
    audio_sr->afe_data = audio_sr->afe_handle->create_from_config(afe_config);

    // 清理内存
    afe_config_free(afe_config);

    // 返回句柄
    return audio_sr;
}

/// @brief 声音识别模块启动
/// @param audio_sr
void Audio_SR_Start(Audio_SR_t *audio_sr)
{
    if (audio_sr->task_flag)
    {
        return;
    }
    audio_sr->task_flag = true;
    // 启动抓获
    xTaskCreateWithCaps(fetch_Task, "fetch", 8 * 1024, audio_sr, 5, &(audio_sr->fetch_handle), MALLOC_CAP_SPIRAM);
    // 启动喂给
    xTaskCreateWithCaps(feed_Task, "feed", 8 * 1024, audio_sr, 5, &(audio_sr->feed_handle), MALLOC_CAP_SPIRAM);
}

/// @brief 声音识别模块停止
/// @param audio_sr
void Audio_SR_Stop(Audio_SR_t *audio_sr)
{
    audio_sr->task_flag = false;
}

/// @brief 缓冲区注册(fetch2encode)
/// @param audio_sr
/// @param buffer
void Audio_SR_Set_Fetch_Out_Buffer(Audio_SR_t *audio_sr, RingbufHandle_t buffer)
{
    audio_sr->fetch_out_buffer = buffer;
}

/// @brief 注册回调函数,当检测到人声状态切换时触发CBvadC,当检测到触发词执行Bwakeup
/// @param audio_sr
/// @param CBvad voice actived detect
/// @param CBwakeup wake up word detect
void Audio_SR_State_Callback_Register(Audio_SR_t *audio_sr, Callback_Vad_t CBvad, Callback_Wakeup_t CBwakeup)
{
    audio_sr->CBvad = CBvad;
    audio_sr->CBwakeup = CBwakeup;
    audio_sr->vadargs = NULL;
    audio_sr->wakeupargs = NULL;
}

Audio_SR_t *audio_sr_error = NULL;

void fetch_error_deal(void)
{
    if (audio_sr_error == NULL)
    {
        return;
    }
    audio_sr_error->afe_handle->reset_buffer(audio_sr_error->afe_data);
}

/// @brief 数据抓取任务
/// @param args
static void fetch_Task(void *args)
{

    // 强转句柄
    Audio_SR_t *audio_sr = (Audio_SR_t *)args;
    //危险危险
    audio_sr_error = audio_sr;
    // 获取afe句柄和实例
    esp_afe_sr_iface_t *afe_handle = audio_sr->afe_handle;
    esp_afe_sr_data_t *afe_data = audio_sr->afe_data;

    afe_fetch_result_t *result;
    int16_t *processed_audio;
    vad_state_t vad_state;
    wakenet_state_t wakeup_state;

    while (audio_sr->task_flag)
    {

        result = afe_handle->fetch(afe_data);
        if (result->ringbuff_free_pct < 0.2)
        {
            afe_handle->reset_buffer(afe_data);
            MY_LOGE("EEEEEEEEEEEEEEEEEEEE,紧急清除buffer");
        }
        processed_audio = result->data;
        vad_state = result->vad_state;
        wakeup_state = result->wakeup_state;
        // printf("[%lf]\n", result->ringbuff_free_pct);
        if (wakeup_state == WAKENET_DETECTED)
        {
            audio_sr->wake_flag = true;
            if (audio_sr->CBwakeup != NULL)
            {
                audio_sr->CBwakeup(audio_sr->wakeupargs);
            }
        }
        if (audio_sr->wake_flag)
        {
            if (com_state == UNCONNECT)
            {
                audio_sr->wake_flag = false;
            }
            if (audio_sr->last_vadstate != vad_state)
            {
                audio_sr->last_vadstate = vad_state;
                if (audio_sr->CBvad != NULL)
                {
                    audio_sr->CBvad(audio_sr->vadargs, vad_state);
                    // printf("负载：%lf\n", result->ringbuff_free_pct);
                }
            }
            if (vad_state == VAD_SPEECH)
            {
                // 将声音放入缓冲区
                if (audio_sr->fetch_out_buffer != NULL && voice_send == VOICE_TO_WEB)
                {
                    xRingbufferSend(audio_sr->fetch_out_buffer, processed_audio, result->data_size, 3000);
                }
            }
        }
        if (result->ringbuff_free_pct < 0.2)
        {
            afe_handle->reset_buffer(audio_sr->afe_data);
            MY_LOGE("DDDDDDDDDDDDDDDDDD,紧急清除buffer");
        }
        vTaskDelay(8);
        if (result->ringbuff_free_pct < 0.2)
        {
            afe_handle->reset_buffer(audio_sr->afe_data);
            MY_LOGE("CCCCCCCCCCCCCCCCCC,紧急清除buffer");
        }
    }
    // 释放资源
    // 删除任务
    vTaskDelete(NULL);
}

/// @brief 数据喂给任务
/// @param args
static void feed_Task(void *args)
{

    // 强转句柄
    Audio_SR_t *audio_sr = (Audio_SR_t *)args;
    // 获取afe句柄和实例
    esp_afe_sr_iface_t *afe_handle = audio_sr->afe_handle;
    esp_afe_sr_data_t *afe_data = audio_sr->afe_data;
    // 计算feed_buff的size
    int feed_chunksize = afe_handle->get_feed_chunksize(afe_data);
    int feed_nch = afe_handle->get_feed_channel_num(afe_data);
    size_t feed_size = feed_chunksize * feed_nch * sizeof(int16_t);
    // 创建feed_buff
    int16_t *feed_buff = (int16_t *)malloc(feed_size);

    while (audio_sr->task_flag)
    {
        Bsp_ES8311_Read_From_Micro((void *)feed_buff, feed_size);
        afe_handle->feed(afe_data, feed_buff);
        vTaskDelay(10);
    }

    // 释放资源
    free(feed_buff);
    // 删除任务
    vTaskDelete(NULL);
}

/// @brief 结束唤醒
/// @param audio_sr
void Audio_SR_Fall_Sleep(Audio_SR_t *audio_sr)
{
    audio_sr->wake_flag = false;
}
