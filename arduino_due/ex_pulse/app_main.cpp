#include "app_main.h"
#include "ex_pulse.h"

#include "pif_log.h"


PifPulse g_pulse;
PifTimerManager g_timer_1ms;


static uint16_t _taskLedToggle(PifTask* p_task)
{
	static BOOL sw = LOW;

	(void)p_task;

   	actLedL(sw);
	sw ^= 1;
}

static uint16_t _taskPulse(PifTask* p_task)
{
	pifPulse_GetPeriod(&g_pulse);
	pifPulse_GetLowLevelTime(&g_pulse);
	pifPulse_GetHighLevelTime(&g_pulse);

	pifLog_Printf(LT_INFO, "P:%luus(%dHz) L:%luus(%d%%) H:%luus(%d%%) R:%lu F:%lu",
			g_pulse._period_1us, (int)pifPulse_GetFrequency(&g_pulse),
			g_pulse._low_level_1us, (int)pifPulse_GetLowLevelDuty(&g_pulse),
			g_pulse._high_level_1us, (int)pifPulse_GetHighLevelDuty(&g_pulse),
			g_pulse.rising_count, g_pulse.falling_count);
    return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(4)) return;

	pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifPulse_Init(&g_pulse, PIF_ID_AUTO)) return;
    pifPulse_SetMeasureMode(&g_pulse, PIF_PMM_PERIOD | PIF_PMM_LOW_LEVEL_TIME | PIF_PMM_HIGH_LEVEL_TIME | PIF_PMM_FALLING_COUNT | PIF_PMM_RISING_COUNT);

	if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLedToggle, NULL, TRUE)) return;	// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 20, _taskPulse, NULL, TRUE)) return;		// 20ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
