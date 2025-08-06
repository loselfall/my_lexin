#include "Bsp_ES8311.h"
// static i2s_comm_mode_t i2s_in_mode = I2S_COMM_MODE_STD;
// static i2s_comm_mode_t i2s_out_mode = I2S_COMM_MODE_STD;
static i2s_keep_t *i2s_keep[I2S_MAX_KEEP];
static const audio_codec_data_if_t *data_if;
static const audio_codec_ctrl_if_t *in_out_ctrl_if;
static const audio_codec_gpio_if_t *gpio_if;
static const audio_codec_if_t *in_out_codec_if;
static esp_codec_dev_handle_t play_record_dev;

static int Bsp_ES8311_I2c_Init(uint8_t port)
{

    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_cfg.sda_io_num = I2C_SDA_PIN;
    i2c_cfg.scl_io_num = I2C_SCL_PIN;
    esp_err_t ret = i2c_param_config(port, &i2c_cfg);
    if (ret != ESP_OK)
    {
        return -1;
    }
    return i2c_driver_install(port, i2c_cfg.mode, 0, 0, 0);
}

static int Bsp_ES8311_I2c_Deinit(uint8_t port)
{

    return i2c_driver_delete(port);
}

// static void Bsp_ES8311_Set_I2s_Mode(i2s_comm_mode_t out_mode, i2s_comm_mode_t in_mode)
// {
//     i2s_in_mode = in_mode;
//     i2s_out_mode = out_mode;
// }

// static void Bsp_ES8311_Clr_I2s_Mode(void)
// {
//     i2s_in_mode = I2S_COMM_MODE_STD;
//     i2s_out_mode = I2S_COMM_MODE_STD;
// }

static int Bsp_ES8311_I2s_Init(uint8_t port)
{
    if (port >= I2S_MAX_KEEP)
    {
        return -1;
    }
    // Already installed
    if (i2s_keep[port])
    {
        return 0;
    }
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(16, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_MCK_PIN,
            .bclk = I2S_BCK_PIN,
            .ws = I2S_DATA_WS_PIN,
            .dout = I2S_DATA_OUT_PIN,
            .din = I2S_DATA_IN_PIN,

        },
    };
    i2s_keep[port] = (i2s_keep_t *)calloc(1, sizeof(i2s_keep_t));
    if (i2s_keep[port] == NULL)
    {
        return -1;
    }
    int ret = i2s_new_channel(&chan_cfg, &i2s_keep[port]->tx_handle, &i2s_keep[port]->rx_handle);
    TEST_ESP_OK(ret);
    ret = i2s_channel_init_std_mode(i2s_keep[port]->tx_handle, &std_cfg);
    TEST_ESP_OK(ret);
    ret = i2s_channel_init_std_mode(i2s_keep[port]->rx_handle, &std_cfg);
    TEST_ESP_OK(ret);
    // For tx master using duplex mode
    i2s_channel_enable(i2s_keep[port]->tx_handle);
    i2s_channel_enable(i2s_keep[port]->rx_handle);
    return ret;
}

static int Bsp_ES8311_I2s_Deinit(uint8_t port)
{

    if (port >= I2S_MAX_KEEP)
    {
        return -1;
    }
    // already installed
    if (i2s_keep[port] == NULL)
    {
        return 0;
    }
    i2s_channel_disable(i2s_keep[port]->tx_handle);
    i2s_channel_disable(i2s_keep[port]->rx_handle);
    i2s_del_channel(i2s_keep[port]->tx_handle);
    i2s_del_channel(i2s_keep[port]->rx_handle);
    free(i2s_keep[port]);
    i2s_keep[port] = NULL;
    return 0;
}

void Bsp_ES8311_Dev_Init(uint8_t Cport, uint8_t Sport)
{

    // data_if
    audio_codec_i2s_cfg_t i2s_cfg = {
        .rx_handle = i2s_keep[0]->rx_handle,
        .tx_handle = i2s_keep[0]->tx_handle,
        .port = Sport};
    data_if = audio_codec_new_i2s_data(&i2s_cfg);
    TEST_ASSERT_NOT_NULL(data_if);

    // ctrl_if
    audio_codec_i2c_cfg_t i2c_cfg = {
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .port = Cport};
    in_out_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    TEST_ASSERT_NOT_NULL(in_out_ctrl_if);

    // gpio_if
    gpio_if = audio_codec_new_gpio();
    TEST_ASSERT_NOT_NULL(gpio_if);

    // ctrl_if + gpio_if = codec if
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
        .ctrl_if = in_out_ctrl_if,
        .gpio_if = gpio_if,
        .pa_pin = PA_EN_PIN,
        .use_mclk = true,
    };
    in_out_codec_if = es8311_codec_new(&es8311_cfg);
    TEST_ASSERT_NOT_NULL(in_out_codec_if);

    //  codec if + data if = dev
    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = in_out_codec_if,
        .data_if = data_if,
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT};

    play_record_dev = esp_codec_dev_new(&dev_cfg);
    TEST_ASSERT_NOT_NULL(play_record_dev);
    int ret = esp_codec_dev_set_out_vol(play_record_dev, 50.0);
    TEST_ESP_OK(ret);
    ret = esp_codec_dev_set_in_gain(play_record_dev, 20.0);
    TEST_ESP_OK(ret);

    esp_codec_dev_sample_info_t fs = {
        .sample_rate = 16000,
        .channel = 1,
        .bits_per_sample = 16,
        .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0),
    };
    ret = esp_codec_dev_open(play_record_dev, &fs);
    TEST_ESP_OK(ret);
}

void Bsp_ES8311_Dev_Deinit(void)
{
    int ret = esp_codec_dev_close(play_record_dev);
    TEST_ESP_OK(ret);
    esp_codec_dev_delete(play_record_dev);

    // Delete codec interface
    audio_codec_delete_codec_if(in_out_codec_if);
    // Delete codec control interface
    audio_codec_delete_ctrl_if(in_out_ctrl_if);
    audio_codec_delete_gpio_if(gpio_if);
    // Delete codec data interface
    audio_codec_delete_data_if(data_if);
}

void Bsp_ES8311_Init(void)
{
    // ��ʼ��I2C��I2S
    int ret = Bsp_ES8311_I2c_Init(I2C_NUM_0);
    TEST_ESP_OK(ret);
    ret = Bsp_ES8311_I2s_Init(I2S_NUM_0);
    TEST_ESP_OK(ret);
    Bsp_ES8311_Dev_Init(I2C_NUM_0, I2S_NUM_0);
}

void Bsp_ES8311_Deinit(void)
{
    Bsp_ES8311_Dev_Deinit();
    Bsp_ES8311_I2c_Deinit(I2C_NUM_0);
    Bsp_ES8311_I2s_Deinit(I2S_NUM_0);
}

void Bsp_ES8311_Read_From_Micro(void *data, int len){
    if (play_record_dev != NULL && len > 0)
    {
        esp_codec_dev_read(play_record_dev, data, len);
    }
}

void Bsp_ES8311_Write_To_Player(void *data, int len){
    if (play_record_dev != NULL && len > 0)
    {
        esp_codec_dev_write(play_record_dev, data, len);
    }
}

void test(void)
{

    uint8_t *data = (uint8_t *)malloc(512);
    Bsp_ES8311_Init();
    // Playback the recording content directly
    while (1)
    {
        Bsp_ES8311_Read_From_Micro(data, 512);
        Bsp_ES8311_Write_To_Player(data, 512);
        vTaskDelay(10);
    }
    free(data);
}

void Bsp_ES8311_Set_Volume(int volume){
    if(volume>=0&&volume<=100){
        esp_codec_dev_set_out_vol(play_record_dev, volume);
    }
}
