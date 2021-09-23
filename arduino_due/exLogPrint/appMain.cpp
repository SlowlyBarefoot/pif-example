#include "appMain.h"
#include "exLogPrint.h"

#include "pifLog.h"


#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              2


PIF_stPulse *g_pstTimer1ms = NULL;


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
	PIF_stComm *pstCommLog;
	PIF_stPulseItem *pstTimer1ms;

	pif_Init(NULL);
	pifLog_Init();

    if (!pifTask_Init(TASK_COUNT)) return;

	pstCommLog = pifComm_Init(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return;	// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstTimer1ms = pifPulse_Init(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;	// 100%

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 500);									// 500ms
}
