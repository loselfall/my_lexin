#ifndef __BSP_LCD_H__
#define __BSP_LCD_H__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_lcd_panel_interface.h"
#include "Bsp_Pwm.h"


#if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
#include "esp_lcd_ili9341.h"
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
#include "esp_lcd_gc9a01.h"
#endif

#if CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
#include "esp_lcd_touch_stmpe610.h"
#endif

// Using SPI2 in the example
#define LCD_HOST  SPI2_HOST
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL  1
#define LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define PIN_NUM_SCLK           47
#define PIN_NUM_MOSI           48
#define PIN_NUM_MISO           18
#define PIN_NUM_LCD_DC         45
#define PIN_NUM_LCD_RST        16
#define PIN_NUM_LCD_CS         21
#define PIN_NUM_BK_LIGHT       40
// The pixel number in horizontal and vertical
#define LCD_H_RES     240
#define LCD_V_RES     320
// Bit number used to represent command and parameter
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

typedef struct
{
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel_handle;

} Bsp_LCD_Handle_t;


Bsp_LCD_Handle_t* Bsp_LCD_Init(void);
void Bsp_LCD_Set_Brightness(int Brightness);
#endif /* __BSP_LCD_H__ */
