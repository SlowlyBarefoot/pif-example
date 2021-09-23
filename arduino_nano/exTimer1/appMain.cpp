#include "appMain.h"
#include "exTimer1.h"


#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              1


PIF_stPulse *g_pstTimer1ms = NULL;


void appSetup()
{
	PIF_stPulseItem *pstTimer1ms;

	pif_Init(NULL);

    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Init(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;	// 100%

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 500);									// 500ms
}
