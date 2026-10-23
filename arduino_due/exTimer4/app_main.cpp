#include "app_port.h"


PifLed g_led_rgb;
PifTimerManager g_timer_1ms;

static PifTimer *s_pstTimer1msRed = NULL;
static PifTimer *s_pstTimer1msYellow = NULL;
static PifTimer *s_pstTimer1msGreen = NULL;


static void _evtLedRedToggle(void *pvIssuer)
{
	static int count = 0;

	(void)pvIssuer;

	pifLed_PartToggle(&g_led_rgb, 1 << 0);

	count++;
	if (count >= 10) {
		count = 0;
		pifTimer_Stop((PifTimer *)pvIssuer);
		pifTimer_Start(s_pstTimer1msYellow, 50);	// 50 * 1ms = 50ms
	}
}

static void _evtLedYellowToggle(void *pvIssuer)
{
	static int count = 0;

	(void)pvIssuer;

	pifLed_PartToggle(&g_led_rgb, 1 << 1);

	count++;
	if (count >= 10) {
		count = 0;
		pifTimer_Stop((PifTimer *)pvIssuer);
		pifTimer_Start(s_pstTimer1msGreen, 100);	// 100 * 1ms = 100ms
	}
}

static void _evtLedGreenToggle(void *pvIssuer)
{
	static int count = 0;

	(void)pvIssuer;

	pifLed_PartToggle(&g_led_rgb, 1 << 2);

	count++;
	if (count >= 10) {
		count = 0;
		pifTimer_Stop((PifTimer *)pvIssuer);
		pifTimer_Start(s_pstTimer1msRed, 20);		// 20 * 1ms = 20ms
	}
}

BOOL appSetup()
{
    s_pstTimer1msRed = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!s_pstTimer1msRed) goto fail;
    pifTimer_AttachEvtFinish(s_pstTimer1msRed, _evtLedRedToggle, s_pstTimer1msRed, FALSE);
    pifTimer_Start(s_pstTimer1msRed, 200);											// 200ms

    s_pstTimer1msYellow = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!s_pstTimer1msYellow) goto fail;
    pifTimer_AttachEvtFinish(s_pstTimer1msYellow, _evtLedYellowToggle, s_pstTimer1msYellow, FALSE);

    s_pstTimer1msGreen = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!s_pstTimer1msGreen) goto fail;
    pifTimer_AttachEvtFinish(s_pstTimer1msGreen, _evtLedGreenToggle, s_pstTimer1msGreen, FALSE);
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
