#include "app_main.h"


PifPulse g_pulse;
PifTimerManager g_timer_1ms;


static uint32_t _taskPulse(PifTask* p_task)
{
	uint16_t period, low_width, high_width;

	(void)p_task;

	period = pifPulse_GetPeriod(&g_pulse);
	low_width = pifPulse_GetLowWidth(&g_pulse);
	high_width = pifPulse_GetHighWidth(&g_pulse);

	pifLog_Printf(LT_INFO, "P:%uus(%dHz) LW:%uus(%d%%) HW:%uus(%d%%) FC:%lu",
			period, 1000000L / period,
			low_width, 100 * low_width / period,
			high_width, 100 * high_width / period,
			g_pulse.falling_count);
    return 0;
}

BOOL appSetup()
{
    if (!pifPulse_Init(&g_pulse, PIF_ID_AUTO)) return FALSE;
    pifPulse_SetMeasureMode(&g_pulse, PIF_PMM_PERIOD | PIF_PMM_COUNT | PIF_PMM_LOW_WIDTH | PIF_PMM_HIGH_WIDTH);

    if (!pifTaskManager_Add(TM_PERIOD, 20000, _taskPulse, NULL, TRUE)) return FALSE;		// 20ms
    return TRUE;
}
