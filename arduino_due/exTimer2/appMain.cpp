#include "appMain.h"
#include "exTimer2.h"


PifTimerManager g_timer_1ms;
PifTimerManager g_timer_100us;


void appSetup()
{
	PifTimer *pstTimer1ms;
	PifTimer *pstTimer100us;

    pif_Init(NULL);

    if (!pifTaskManager_Init(2)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

    if (!pifTimerManager_Init(&g_timer_100us, PIF_ID_AUTO, 100, 1)) return;	// 100us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedRedToggle, NULL);
    if (!pifTimer_Start(pstTimer1ms, 5)) return;							// 5 * 1ms = 5ms

    pstTimer100us = pifTimerManager_Add(&g_timer_100us, TT_REPEAT);
    if (!pstTimer100us) return;
    pifTimer_AttachEvtFinish(pstTimer100us, evtLedYellowToggle, NULL);
    if (!pifTimer_Start(pstTimer100us, 5)) return;							// 5 * 100us = 500us
}
