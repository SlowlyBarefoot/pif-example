#include "appMain.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;


static void _evtLedToggle(void *pvIssuer)
{
	static BOOL sw = FALSE;
	static int count = 19;

	(void)pvIssuer;

	pifLed_AllChange(&g_led_l, sw);
	sw ^= 1;

	pifLog_Printf(LT_INFO, "LED: %u", sw);

	if (count) count--;
	else {
	    pifLog_PrintInBuffer();
	    count = 19;
	}
}

BOOL appSetup()
{
	PifTimer *pstTimer1ms;

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return FALSE;
    pifTimer_AttachEvtFinish(pstTimer1ms, _evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);											// 500ms

    pifLog_Disable();
    return TRUE;
}
