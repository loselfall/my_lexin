#include "Bsp_Display.h"
#define LCD_DRAW_BUFF_HEIGHT (50)
#define LCD_DRAW_BUFF_DOUBLE (1)
#define EMOJI_MAP_SIZE 21
LV_FONT_DECLARE(font_puhui_14_1)
LV_FONT_DECLARE(font_puhui_16_4)
LV_FONT_DECLARE(font_puhui_20_4)
LV_FONT_DECLARE(font_emoji_64)

typedef struct 
{
    char* key;
    char* value;
    /* data */
}emoji_kv;


static lv_obj_t *t_label;
static lv_obj_t *e_label;
static lv_obj_t *scr;
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
    scr = lv_scr_act();

    lvgl_port_lock(0);
    t_label = lv_label_create(scr);
    e_label = lv_label_create(scr);
    lvgl_port_unlock();

}

void Bsp_Display_Text(char*str)
{
        
    
    lvgl_port_lock(0);
    

    
    lv_obj_set_width(t_label, LCD_H_RES);
    lv_obj_align(t_label,LV_ALIGN_CENTER,0,20);
    lv_obj_set_style_text_align(t_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(t_label,&font_puhui_20_4,0);
    
    lv_label_set_text(t_label,str);
    lv_label_set_long_mode(t_label,LV_LABEL_LONG_MODE_WRAP);

    lvgl_port_unlock();
}

void Bsp_Display_Button(void){
     
    lv_obj_t *scr = lv_scr_act();
    lvgl_port_lock(0);
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_t *label = lv_label_create(btn);
    lv_obj_set_style_text_font(label,&font_puhui_20_4,0);
    lv_label_set_text_static(label, "Hi, 乐鑫!");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    //lv_obj_add_event_cb(btn, _app_button_cb, LV_EVENT_CLICKED, NULL);
    lvgl_port_unlock();
    /* Task unlock */
}

// # map to emoji
static emoji_kv emoji_mapping[EMOJI_MAP_SIZE] = {
{"neutral"       , "😶"},    
{"happy"         , "🙂"},      
{"laughing"      , "😆"},   
{"funny"         , "😂"},      
{"sad"           , "😔"},        
{"angry"         , "😠"},      
{"crying"        , "😭"},     
{"loving"        , "😍"},     
{"embarrassed"   , "😳"}, 
{"surprised"     , "😯"},  
{"shocked"       , "😱"},    
{"thinking"      , "🤔"},   
{"winking"       , "😉"},    
{"cool"          , "😎"},       
{"relaxed"       , "😌"},    
{"delicious"     , "🤤"},  
{"kissy"         , "😘"},      
{"confident"     , "😏"},  
{"sleepy"        , "😴"},     
{"silly"         , "😜"},      
{"confused"      , "🙄"},  
}; 
//    "neutral"       , 0x1f636     # 😶
//    "happy"         , 0x1f642     # 🙂
//    "laughing"      , 0x1f606,    # 😆
//    "funny"         , 0x1f602     # 😂
//    "sad"           , 0x1f614     # 😔
//    "angry"         , 0x1f620     # 😠
//    "crying"        , 0x1f62d     # 😭
//    "loving"        , 0x1f60d     # 😍
//    "embarrassed"   , 0x1f633     # 😳
//    "surprised"     , 0x1f62ff,   # 😯
//    "shocked"       , 0x1f631     # 😱
//    "thinking"      , 0x1f914,    # 🤔
//    "winking"       , 0x1f609     # 😉
//    "cool"          , 0x1f60e     # 😎
//    "relaxed"       , 0x1f60c     # 😌
//    "delicious"     , 0x1f9244,   # 🤤
//    "kissy"         , 0x1f618     # 😘
//    "confident"     , 0x1f60ff,   # 😏
//    "sleepy"        , 0x1f634     # 😴
//    "silly"         , 0x1f61c     # 😜
//    "confused"      , 0x1f644     # 🙄
// }

void Bsp_Display_emoji(char*str)
{
    char* emoji = "😶";
    for(int i=0;i<EMOJI_MAP_SIZE;i++){
        if(strcmp(emoji_mapping[i].key,str)==0){
            emoji = emoji_mapping[i].value;
            break;
        }
    }
    lvgl_port_lock(0);

    lv_obj_set_width(e_label, LCD_H_RES);
    lv_obj_align(e_label,LV_ALIGN_TOP_MID,0,50);
    lv_obj_set_style_text_align(e_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(e_label,font_emoji_64_init(),0);
    
    lv_label_set_text(e_label,emoji);
    lv_label_set_long_mode(e_label,LV_LABEL_LONG_MODE_WRAP);

    lvgl_port_unlock();
}