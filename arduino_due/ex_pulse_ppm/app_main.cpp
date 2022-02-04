#include "app_main.h"
#include "ex_pulse_ppm.h"

#include "pif_log.h"


PifPulse g_pulse;
PifTimerManager g_timer_1ms;


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
	uint16_t value[8];

	(void)p_task;

	if (pifPulse_GetPositionModulation(&g_pulse, value)) {
		pifLog_Printf(LT_INFO, "1:%5u 2:%5u 3:%5u 4:%5u 5:%5u 6:%5u 7:%5u 8:%5u",
				value[0], value[1], value[2], value[3],	value[4], value[5], value[6], value[7]);
	}
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
    if (!pifPulse_SetPositionModulation(&g_pulse, 8, 2700)) return;

	if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLedToggle, NULL, TRUE)) return;	// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 20, _taskPulse, NULL, TRUE)) return;		// 20ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
