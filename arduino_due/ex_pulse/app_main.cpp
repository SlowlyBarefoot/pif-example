#include "app_main.h"
#include "ex_pulse.h"

#include "core/pif_log.h"


PifPulse g_pulse;
PifTimerManager g_timer_1ms;


static void _evtPulseEdge(PifPulseState state, PifIssuerP p_issuer)
{
	PifTask* p_task = (PifTask*)p_issuer;

	(void)state;

	pifTask_SetTrigger(p_task);
}

static uint16_t _taskLedToggle(PifTask* p_task)
{
	static BOOL sw = LOW;

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

	pifLog_Printf(LT_INFO, "P:%uus(%dHz) LW:%uus(%d%%) HW:%uus(%d%%) RC:%lu",
			period, 1000000L / period,
			low_width, 100 * low_width / period,
			high_width, 100 * high_width / period,
			g_pulse.falling_count);
    return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	PifTask* p_task;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(4)) return;

	pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

	p_task = pifTaskManager_Add(TM_EXTERNAL_ORDER, 0, _taskPulse, NULL, FALSE);
    if (!p_task) return;

    if (!pifPulse_Init(&g_pulse, PIF_ID_AUTO)) return;
    pifPulse_SetMeasureMode(&g_pulse, PIF_PMM_PERIOD | PIF_PMM_COUNT | PIF_PMM_LOW_WIDTH | PIF_PMM_HIGH_WIDTH);
    pifPulse_AttachEvtEdge(&g_pulse, _evtPulseEdge, p_task);

	if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLedToggle, NULL, TRUE)) return;	// 100ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
