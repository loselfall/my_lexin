#ifndef __COM_STATE_H__
#define __COM_STATE_H__

#include <stdio.h>
#include "Com_Debug.h"
#include "Bsp_WS2812.h"

typedef enum
{
    UNCONNECT,
    IDLE,
    LISTEN,
    SPEAK,
} COM_STATE;

typedef enum
{
    VOICE_TO_SR,
    VOICE_TO_WEB
} VOICE_SEND;

/// @brief 切换状态必须使用本函数
/// @param new_state 
void Com_State_Change(COM_STATE new_state);

extern VOICE_SEND voice_send;
extern COM_STATE com_state;
#endif /* __COM_STATE_H__ */
