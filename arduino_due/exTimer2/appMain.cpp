#include "appMain.h"
#include "exTimer2.h"


PifTimerManager *g_pstTimer1ms = NULL;
PifTimerManager *g_pstTimer100us = NULL;


void appSetup()
{
	PifTimer *pstTimer1ms;
	PifTimer *pstTimer100us;

    pif_Init(NULL);

    if (!pifTaskManager_Init(2)) return;

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 1);			// 1000us
    if (!g_pstTimer1ms) return;

    g_pstTimer100us = pifTimerManager_Create(PIF_ID_AUTO, 100, 1);			// 100us
    if (!g_pstTimer100us) return;

    pstTimer1ms = pifTimerManager_Add(g_pstTimer1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedRedToggle, NULL);
    if (!pifTimer_Start(pstTimer1ms, 5)) return;							// 5 * 1ms = 5ms

    pstTimer100us = pifTimerManager_Add(g_pstTimer100us, TT_REPEAT);
    if (!pstTimer100us) return;
    pifTimer_AttachEvtFinish(pstTimer100us, evtLedYellowToggle, NULL);
    if (!pifTimer_Start(pstTimer100us, 5)) return;							// 5 * 100us = 500us
}
