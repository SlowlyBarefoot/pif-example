#include "appMain.h"
#include "exLogPrint.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;


static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	pifLed_AllChange(&g_led_l, sw);
	sw ^= 1;

	pifLog_Printf(LT_INFO, "LED: %u", sw);
}

BOOL appSetup()
{
	PifTimer *pstTimer1ms;

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return FALSE;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);											// 500ms
    return TRUE;
}
