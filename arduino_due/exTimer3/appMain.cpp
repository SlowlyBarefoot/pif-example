#include "appMain.h"


PifLed g_led_rgb;
PifTimerManager g_timer_1ms;

static PifTimer *s_pstTimer1msRed = NULL;
static PifTimer *s_pstTimer1msYellow = NULL;
static PifTimer *s_pstTimer1msGreen = NULL;


static void _evtLedRedToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_PartToggle(&g_led_rgb, 1 << 0);

	pifTimer_Start(s_pstTimer1msYellow, 500);	// 500 * 1ms = 0.5sec
}

static void _evtLedYellowToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_PartToggle(&g_led_rgb, 1 << 1);

	pifTimer_Start(s_pstTimer1msGreen, 1000);	// 1000 * 1ms = 1sec
}

static void _evtLedGreenToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_PartToggle(&g_led_rgb, 1 << 2);

	pifTimer_Start(s_pstTimer1msRed, 200);		// 200 * 1ms = 0.2sec
}

BOOL appSetup()
{
    s_pstTimer1msRed = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_pstTimer1msRed) return FALSE;
    pifTimer_AttachEvtFinish(s_pstTimer1msRed, _evtLedRedToggle, NULL);
    pifTimer_Start(s_pstTimer1msRed, 200);								    // 200ms

    s_pstTimer1msYellow = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_pstTimer1msYellow) return FALSE;
    pifTimer_AttachEvtFinish(s_pstTimer1msYellow, _evtLedYellowToggle, NULL);

    s_pstTimer1msGreen = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_pstTimer1msGreen) return FALSE;
    pifTimer_AttachEvtFinish(s_pstTimer1msGreen, _evtLedGreenToggle, NULL);
    return TRUE;
}
