#include "appMain.h"
#include "main.h"


PifPulse *g_pstTimer1ms = NULL;


void appSetup()
{
	PifPulseItem *pstTimer1ms;

	pif_Init(NULL);

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_REPEAT);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 500);									// 500ms
}
