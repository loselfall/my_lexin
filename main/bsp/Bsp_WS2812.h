#ifndef __BSP_WS2812_H__
#define __BSP_WS2812_H__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"

#define LED_STRIP_USE_DMA  0
#define LED_STRIP_LED_COUNT 2
#define LED_STRIP_MEMORY_BLOCK_WORDS 0 

typedef enum {
    RED,
    WHITE,
    GREEN,
    BLUE,
}LIGHT_COLOR;

// GPIO assignment
#define LED_STRIP_GPIO_PIN  46

// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

void Bsp_WS2812_Init(void);
void Bsp_WS2812_Open(void);
void Bsp_WS2812_Close(void);
void Bsp_WS2812_Set(LIGHT_COLOR color);


#endif /* __Bsp_WS2812_H__ */
