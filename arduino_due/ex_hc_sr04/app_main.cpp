#include "app_port.h"


PifHcSr04 g_hcsr04;
PifTimerManager g_timer_1ms;


static void _evtHcSr04Distance(int32_t distance)
{
	pifLog_Printf(LT_INFO, "Distance = %dcm", distance);
}

BOOL appSetup()
{
	g_hcsr04.evt_read = _evtHcSr04Distance;
	if (!pifHcSr04_StartTrigger(&g_hcsr04, 70)) goto fail;		// 70ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            ex_hc_sr04            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
