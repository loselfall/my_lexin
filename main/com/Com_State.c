#include "Com_State.h"

char *statemap[] =
    {
        "UNCONNECT",
        "IDLE",
        "LISTEN",
        "SPEAK",
};

char *voicemap[] =
    {
        "VOICE_TO_SR",
        "VOICE_TO_WEB",
};

VOICE_SEND voice_send = VOICE_TO_SR;

COM_STATE com_state = UNCONNECT;

__weak_symbol void open_websocket_listen_weak(void){

}

__weak_symbol void stop_websocket_listen_weak(void){

}

void Com_State_Change(COM_STATE new_state)
{
    COM_STATE last_state;
    if (new_state != com_state)
    {
        last_state = com_state;
        com_state = new_state;
        switch (com_state)
        {
        case IDLE:
        stop_websocket_listen_weak();
        open_websocket_listen_weak();
        case UNCONNECT:;
        case SPEAK:
            voice_send = VOICE_TO_SR;
            break;
        case LISTEN:
            voice_send = VOICE_TO_WEB;

            break;
        }
        Bsp_WS2812_Set((LIGHT_COLOR)new_state);
       MY_LOGW("state[%s->%s],voice[%s]\r\n",statemap[last_state] ,statemap[com_state],voicemap[voice_send]);
    }
}
