#include "appMain.h"
#include "main.h"


#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              1


PIF_stPulse *g_pstTimer1ms = NULL;


void appSetup()
{
	PIF_stPulseItem *pstTimer1ms;

	pif_Init(NULL);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);				// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_Add(TM_enRatio, 100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;	// 100%

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 500);											// 500ms
}
