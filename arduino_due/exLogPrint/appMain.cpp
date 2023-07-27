#include "appMain.h"


PifTimerManager g_timer_1ms;
PifTimer* g_timer_led;


BOOL appSetup()
{
    pifTimer_Start(g_timer_led, 500);		// 500ms
    return TRUE;
}
