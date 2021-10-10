#include "appMain.h"
#include "exTimer2.h"


PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stPulse *g_pstTimer100us = NULL;


void appSetup()
{
	PIF_stPulseItem *pstTimer1ms;
	PIF_stPulseItem *pstTimer100us;

    pif_Init(NULL);

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);							// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;		// 100%

    g_pstTimer100us = pifPulse_Create(PIF_ID_AUTO, 100);						// 100us
    if (!g_pstTimer100us) return;
    if (!pifPulse_AttachTask(g_pstTimer100us, TM_RATIO, 100, TRUE)) return;	// 100%

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, evtLedRedToggle, NULL);
    if (!pifPulse_StartItem(pstTimer1ms, 5)) return;							// 5 * 1ms = 5ms

    pstTimer100us = pifPulse_AddItem(g_pstTimer100us, PT_enRepeat);
    if (!pstTimer100us) return;
    pifPulse_AttachEvtFinish(pstTimer100us, evtLedYellowToggle, NULL);
    if (!pifPulse_StartItem(pstTimer100us, 5)) return;							// 5 * 100us = 500us
}
