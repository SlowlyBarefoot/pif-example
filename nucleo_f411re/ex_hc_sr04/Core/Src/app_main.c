#include "app_main.h"
#include "main.h"

#include "core/pif_log.h"


PifComm g_comm_log;
PifHcSr04 g_hcsr04;
PifTimerManager g_timer_1ms;


static void _evtHcSr04Distance(int32_t distance)
{
	pifLog_Printf(LT_INFO, "Distance = %dcm", distance);
}

void appSetup(PifActTimer1us act_timer1us)
{
	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(5)) return;

	pifLog_Init();

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

    if (!pifHcSr04_Init(&g_hcsr04, PIF_ID_AUTO)) return;
	g_hcsr04.act_trigger = actHcSr04Trigger;
	g_hcsr04.evt_distance = _evtHcSr04Distance;
	if (!pifHcSr04_StartTrigger(&g_hcsr04, 70)) return;								// 70ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
