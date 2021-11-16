#include "appMain.h"
#include "exLogPrint.h"

#include "pif_log.h"


PifTimerManager g_timer_1ms;


static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	actLedL(sw);
	sw ^= 1;

	pifLog_Printf(LT_INFO, "LED: %u", sw);
}

void appSetup()
{
	static PifComm s_comm_log;
	PifTimer *pstTimer1ms;

	pif_Init(NULL);

    if (!pifTaskManager_Init(2)) return;

	pifLog_Init();

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);										// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
