#include "appMain.h"
#include "exLogBuffer.h"

#include "core/pif_log.h"


#define LOG_BUFFER_SIZE			0x200


PifTimerManager g_timer_1ms;

static uint8_t s_aucLog[LOG_BUFFER_SIZE];


static void _evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;
	static int count = 19;

	(void)pvIssuer;

	actLedL(sw);
	sw ^= 1;

	pifLog_Printf(LT_INFO, "LED: %u", sw);

	if (count) count--;
	else {
	    pifLog_PrintInBuffer();
	    count = 19;
	}
}

void appSetup()
{
	static PifComm s_comm_log;
	PifTimer *pstTimer1ms;

	pif_Init(NULL);

    if (!pifTaskManager_Init(2)) return;

    if (!pifLog_InitStatic(LOG_BUFFER_SIZE, s_aucLog)) return;

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, _evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);										// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));

    pifLog_Disable();
}
