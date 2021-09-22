#include "appMain.h"
#include "exLogBuffer.h"

#include "pifLog.h"


#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              2

#define LOG_BUFFER_SIZE			0x200


PIF_stPulse *g_pstTimer1ms = NULL;

static uint8_t s_aucLog[LOG_BUFFER_SIZE];


static void _evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;
	static int count = 19;

	(void)pvIssuer;

	actLedL(sw);
	sw ^= 1;

	pifLog_Printf(LT_enInfo, "LED: %u", sw);

	if (count) count--;
	else {
	    pifLog_PrintInBuffer();
	    count = 19;
	}
}

void appSetup()
{
	PIF_stComm *pstCommLog;
	PIF_stPulseItem *pstTimer1ms;

	pif_Init(NULL);
    if (!pifLog_InitStatic(LOG_BUFFER_SIZE, s_aucLog)) return;

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_Add(TM_enPeriodMs, 1, pifComm_Task, pstCommLog, TRUE)) return;		// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);				// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_Add(TM_enRatio, 100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;	// 100%

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, _evtLedToggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 500);											// 500ms

    pifLog_Disable();
}
