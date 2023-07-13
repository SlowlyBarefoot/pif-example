#include "app_main.h"
#include "main.h"

#include "core/pif_log.h"


PifComm g_comm_log;
PifPulse g_pulse;
PifTimerManager g_timer_1ms;


static uint16_t _taskLedToggle(PifTask* p_task)
{
	static BOOL sw = FALSE;

	(void)p_task;

   	actLedL(sw);
	sw ^= 1;
    return 0;
}

static uint16_t _taskPulse(PifTask* p_task)
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

void appSetup(PifActTimer1us act_timer1us)
{
	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(4)) return;

	pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifPulse_Init(&g_pulse, PIF_ID_AUTO)) return;
    pifPulse_SetMeasureMode(&g_pulse, PIF_PMM_PERIOD | PIF_PMM_COUNT | PIF_PMM_LOW_WIDTH | PIF_PMM_HIGH_WIDTH);

	if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLedToggle, NULL, TRUE)) return;	// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 20, _taskPulse, NULL, TRUE)) return;		// 20ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
