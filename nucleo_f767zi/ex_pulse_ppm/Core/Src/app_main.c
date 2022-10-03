#include "app_main.h"
#include "main.h"

#include "core/pif_log.h"


PifComm g_comm_log;
PifPulse g_pulse;
PifTimerManager g_timer_1ms;

static uint16_t s_value[8];

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
	(void)p_task;

	pifLog_Printf(LT_INFO, "1:%5u 2:%5u 3:%5u 4:%5u 5:%5u 6:%5u 7:%5u 8:%5u",
			s_value[0], s_value[1], s_value[2], s_value[3],	s_value[4], s_value[5], s_value[6], s_value[7]);
    return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(4)) return;

	pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifPulse_Init(&g_pulse, PIF_ID_AUTO)) return;
    pifPulse_SetPositionMode(&g_pulse, 8, 2700, s_value);

	if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLedToggle, NULL, TRUE)) return;	// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 20, _taskPulse, NULL, TRUE)) return;		// 20ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
