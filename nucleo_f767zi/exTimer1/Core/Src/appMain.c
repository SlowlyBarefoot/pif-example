#include "appMain.h"
#include "main.h"


PifTimerManager g_timer_1ms;


void appSetup()
{
	PifTimer *pstTimer1ms;

	pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);										// 500ms
}
