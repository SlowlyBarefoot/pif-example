#include "appMain.h"
#include "exTimer3.h"

#include "pif_led.h"


PifTimerManager *g_pstTimer1ms = NULL;

static PifLed *s_pstLedRGB = NULL;
static PifTimer *s_pstTimer1msRed = NULL;
static PifTimer *s_pstTimer1msYellow = NULL;
static PifTimer *s_pstTimer1msGreen = NULL;


static void _evtLedRedToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 0);

	pifTimer_Start(s_pstTimer1msYellow, 500);	// 500 * 1ms = 0.5sec
}

static void _evtLedYellowToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 1);

	pifTimer_Start(s_pstTimer1msGreen, 1000);	// 1000 * 1ms = 1sec
}

static void _evtLedGreenToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 2);

	pifTimer_Start(s_pstTimer1msRed, 200);		// 200 * 1ms = 0.2sec
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return;

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 3);			// 1000us
    if (!g_pstTimer1ms) return;

    s_pstTimer1msRed = pifTimerManager_Add(g_pstTimer1ms, TT_ONCE);
    if (!s_pstTimer1msRed) return;
    pifTimer_AttachEvtFinish(s_pstTimer1msRed, _evtLedRedToggle, NULL);
    pifTimer_Start(s_pstTimer1msRed, 200);								    // 200ms

    s_pstTimer1msYellow = pifTimerManager_Add(g_pstTimer1ms, TT_ONCE);
    if (!s_pstTimer1msYellow) return;
    pifTimer_AttachEvtFinish(s_pstTimer1msYellow, _evtLedYellowToggle, NULL);

    s_pstTimer1msGreen = pifTimerManager_Add(g_pstTimer1ms, TT_ONCE);
    if (!s_pstTimer1msGreen) return;
    pifTimer_AttachEvtFinish(s_pstTimer1msGreen, _evtLedGreenToggle, NULL);

    s_pstLedRGB = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedRGBState);
    if (!s_pstLedRGB) return;
}
