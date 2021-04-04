#include "appMain.h"
#include "exLogBuffer.h"

#include "pifLog.h"


#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              1

#define LOG_BUFFER_SIZE			0x200


PIF_stPulse *g_pstTimer1ms = NULL;

static char s_acLog[LOG_BUFFER_SIZE];


static void _evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;
	static int count = 9;

	(void)pvIssuer;

	actLedL(sw);
	sw ^= 1;

	pifLog_Printf(LT_enInfo, "%d", sw);

	if (count) count--;
	else {
	    pifLog_PrintInBuffer();
	    count = 9;
	}
}

void appSetup()
{
	PIF_stPulseItem *pstTimer1ms;

	pif_Init(NULL);

    if (!pifLog_InitStatic(LOG_BUFFER_SIZE, s_acLog)) return;
	pifLog_AttachActPrint(actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, _evtLedToggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 1000);									// 1000ms

    pifLog_Disable();
}
