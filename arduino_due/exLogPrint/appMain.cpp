#include "appMain.h"
#include "exLogPrint.h"

#include "pifLog.h"


PifPulse *g_pstTimer1ms = NULL;


static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	actLedL(sw);
	sw ^= 1;

	pifLog_Printf(LT_enInfo, "LED: %u", sw);
}

void appSetup()
{
	PifComm *pstCommLog;
	PifPulseItem *pstTimer1ms;

	pif_Init(NULL);
	pifLog_Init();

	pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;	// 100%

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_REPEAT);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 500);									// 500ms
}
