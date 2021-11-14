#include "appMain.h"
#include "exLogBuffer.h"

#include "pif_log.h"


#define LOG_BUFFER_SIZE			0x200


PifTimerManager *g_pstTimer1ms = NULL;

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
	PifComm *pstCommLog;
	PifTimer *pstTimer1ms;

	pif_Init(NULL);

    if (!pifTaskManager_Init(2)) return;

    if (!pifLog_InitStatic(LOG_BUFFER_SIZE, s_aucLog)) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 1);		// 1000us
    if (!g_pstTimer1ms) return;

    pstTimer1ms = pifTimerManager_Add(g_pstTimer1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, _evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);									// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(g_pstTimer1ms));

    pifLog_Disable();
}
