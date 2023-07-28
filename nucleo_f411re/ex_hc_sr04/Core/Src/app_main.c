#include "app_main.h"


PifHcSr04 g_hcsr04;
PifTimerManager g_timer_1ms;


static void _evtHcSr04Distance(int32_t distance)
{
	pifLog_Printf(LT_INFO, "Distance = %dcm", distance);
}

BOOL appSetup()
{
	g_hcsr04.evt_read = _evtHcSr04Distance;
	if (!pifHcSr04_StartTrigger(&g_hcsr04, 70)) return FALSE;		// 70ms
	return TRUE;
}
