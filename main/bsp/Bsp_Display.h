#ifndef __BSP_DISPLAY_H__
#define __BSP_DISPLAY_H__


#include "esp_err.h"
#include "Com_Debug.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

#include "Bsp_LCD.h"

void Bsp_Display_Init(Bsp_LCD_Handle_t*bsp_lcd);

void Bsp_Display_Text(char*str);


#endif /* __BSP_DISPLAY_H__ */
