#include "appMain.h"
#include "exTimer2.h"


#define PULSE_COUNT         	2
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              1


PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stPulse *g_pstTimer100us = NULL;


void appSetup()
{
	PIF_stPulseItem *pstTimer1ms;
	PIF_stPulseItem *pstTimer100us;

    pif_Init();

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    g_pstTimer100us = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 100);		// 100us
    if (!g_pstTimer100us) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, evtLedRedToggle, NULL);
    if (!pifPulse_StartItem(pstTimer1ms, 500)) return;						// 500ms

    pstTimer100us = pifPulse_AddItem(g_pstTimer100us, PT_enRepeat);
    if (!pstTimer100us) return;
    pifPulse_AttachEvtFinish(pstTimer100us, evtLedYellowToggle, NULL);
    if (!pifPulse_StartItem(pstTimer100us, 5000)) return;					// 5000 * 100us = 500ms
}
