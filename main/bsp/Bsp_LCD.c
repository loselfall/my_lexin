#include "Bsp_LCD.h"

Bsp_LCD_Handle_t* Bsp_LCD_Init(void)
{
    Bsp_LCD_Handle_t* lcd_handle=malloc(sizeof(Bsp_LCD_Handle_t));
    lcd_handle->io_handle = NULL;
    lcd_handle->panel_handle = NULL;
    // 背光引脚配置信息
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT};
    // 设置背光引脚配置
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    // SPI引脚配置信息
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_SCLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };
    // 初始化SPI
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // lcd引脚IO相关
    lcd_handle->io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_LCD_DC,
        .cs_gpio_num = PIN_NUM_LCD_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .user_ctx = NULL,
        .on_color_trans_done = NULL,
    };
    // 将LCD附着到SPIbus上
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &lcd_handle->io_handle));

    // lcd面板驱动相关
    lcd_handle->panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    // 安装面板驱动
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_handle->io_handle, &panel_config, &lcd_handle->panel_handle));

    // 面板复位
    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_handle->panel_handle));
    // 面板初始化
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_handle->panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(lcd_handle->panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd_handle->panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle->panel_handle, true));
    // 打开背光
    //gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL);
    Bsp_Pwm_Init();
    return lcd_handle;
}

void Bsp_LCD_Set_Brightness(int Brightness){
    
    int duty = (int)(LEDC_DUTY_MAX*Brightness/100);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

