#include "app_port.h"


PifTimerManager g_timer_1ms;
PifTimerManager g_timer_100us;
PifTimer* g_timer_red;
PifTimer* g_timer_yellow;


BOOL appSetup()
{
    if (!pifTimer_Start(g_timer_red, 5)) goto fail;			// 5 * 1ms = 5ms

    if (!pifTimer_Start(g_timer_yellow, 5)) goto fail;		// 5 * 100us = 500us
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
