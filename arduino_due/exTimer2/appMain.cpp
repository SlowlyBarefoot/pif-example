#include "appMain.h"
#include "exTimer2.h"


PifPulse *g_pstTimer1ms = NULL;
PifPulse *g_pstTimer100us = NULL;


void appSetup()
{
	PifPulseItem *pstTimer1ms;
	PifPulseItem *pstTimer100us;

    pif_Init(NULL);

    if (!pifTaskManager_Init(2)) return;

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000, 1);						// 1000us
    if (!g_pstTimer1ms) return;

    g_pstTimer100us = pifPulse_Create(PIF_ID_AUTO, 100, 1);						// 100us
    if (!g_pstTimer100us) return;

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_REPEAT);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, evtLedRedToggle, NULL);
    if (!pifPulse_StartItem(pstTimer1ms, 5)) return;							// 5 * 1ms = 5ms

    pstTimer100us = pifPulse_AddItem(g_pstTimer100us, PT_REPEAT);
    if (!pstTimer100us) return;
    pifPulse_AttachEvtFinish(pstTimer100us, evtLedYellowToggle, NULL);
    if (!pifPulse_StartItem(pstTimer100us, 5)) return;							// 5 * 100us = 500us
}
