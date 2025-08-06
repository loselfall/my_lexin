#include "Bsp_Display.h"
#define LCD_DRAW_BUFF_HEIGHT (50)
#define LCD_DRAW_BUFF_DOUBLE (1)
static lv_display_t *lvgl_disp = NULL;
static esp_err_t app_lvgl_init(Bsp_LCD_Handle_t *bsp_lcd)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };

    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    /* Add LCD screen */
    MY_LOGI("Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = bsp_lcd->io_handle,
        .panel_handle = bsp_lcd->panel_handle,
        .buffer_size = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT,
        .double_buffer = LCD_DRAW_BUFF_DOUBLE,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .rotation = {
            .swap_xy = false,
            .mirror_x = true,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = true,
#endif
        }};

    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
    // /* Add touch input (for selected screen) */
    // const lvgl_port_touch_cfg_t touch_cfg = {
    // .disp = lvgl_disp,
    // .handle = touch_handle,
    // };
    // lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}

void Bsp_Display_Init(Bsp_LCD_Handle_t *bsp_lcd)
{
    app_lvgl_init(bsp_lcd);
}

void Bsp_Display_Text(char*str)
{
        
    lv_obj_t *scr = lv_scr_act();
    lvgl_port_lock(0);
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_width(label, LCD_H_RES);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label,str);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 20);

    // /* Button */
    // lv_obj_t *btn = lv_btn_create(scr);
    // label = lv_label_create(btn);
    // lv_label_set_text_static(label, "Rotate screen");
    // lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    //lv_obj_add_event_cb(btn, _app_button_cb, LV_EVENT_CLICKED, NULL);
    /* Task unlock */
    lvgl_port_unlock();
}
