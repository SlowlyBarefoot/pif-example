#include "appMain.h"
#include "exTimer1.h"


PifTimerManager *g_pstTimer1ms = NULL;


void appSetup()
{
	PifTimer *pstTimer1ms;

	pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return;

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 1);		// 1000us
    if (!g_pstTimer1ms) return;

    pstTimer1ms = pifTimerManager_Add(g_pstTimer1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);									// 500ms
}
