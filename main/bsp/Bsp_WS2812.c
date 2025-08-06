#include "Bsp_WS2812.h"

static const char *TAG = "Ws2812";
static led_strip_handle_t led_strip;
static bool light = true;
void Bsp_WS2812_Init(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO_PIN,                        // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_LED_COUNT,                             // The number of LEDs in the strip,
        .led_model = LED_MODEL_WS2812,                               // LED strip model
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color order of the strip: GRB
        .flags = {
            .invert_out = false, // don't invert the output signal
        }};

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,                    // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ,             // RMT counter clock frequency
        .mem_block_symbols = LED_STRIP_MEMORY_BLOCK_WORDS, // the memory block size used by the RMT channel
        .flags = {
            .with_dma = LED_STRIP_USE_DMA, // Using DMA can improve performance when driving more LEDs
        }};

    // LED Strip object handle

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
}

void Bsp_WS2812_Open(void)
{
    light = true;
}

void Bsp_WS2812_Set(LIGHT_COLOR color)
{
    if (light == false)
    {
        return;
    }
    for (int i = 0; i < LED_STRIP_LED_COUNT; i++)
    {
        switch (color)
        {

        case WHITE:
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 20, 20, 20));
            break;
        case RED:
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 50, 0, 0));
            break;
        case BLUE:
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 0, 0, 50));
            break;
        case GREEN:
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 0, 50, 0));
            break;
        }
    }
    led_strip_refresh(led_strip);
}

void Bsp_WS2812_Close(void)
{
    ESP_LOGI("light","关灯");
    light = false;
    led_strip_clear(led_strip);
}
