#include "appMain.h"
#include "exTimer4.h"

#include "pif_led.h"


PifTimerManager *g_pstTimer1ms = NULL;

static PifLed *s_pstLedRGB = NULL;
static PifTimer *s_pstTimer1msRed = NULL;
static PifTimer *s_pstTimer1msYellow = NULL;
static PifTimer *s_pstTimer1msGreen = NULL;


static void _evtLedRedToggle(void *pvIssuer)
{
	static int count = 0;

	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 0);

	count++;
	if (count >= 10) {
		count = 0;
		pifTimer_Stop((PifTimer *)pvIssuer);
		pifTimer_Start(s_pstTimer1msYellow, 500);	// 500 * 1ms = 0.5sec
	}
}

static void _evtLedYellowToggle(void *pvIssuer)
{
	static int count = 0;

	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 1);

	count++;
	if (count >= 10) {
		count = 0;
		pifTimer_Stop((PifTimer *)pvIssuer);
		pifTimer_Start(s_pstTimer1msGreen, 1000);	// 1000 * 1ms = 1sec
	}
}

static void _evtLedGreenToggle(void *pvIssuer)
{
	static int count = 0;

	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 2);

	count++;
	if (count >= 10) {
		count = 0;
		pifTimer_Stop((PifTimer *)pvIssuer);
		pifTimer_Start(s_pstTimer1msRed, 200);		// 200 * 1ms = 0.2sec
	}
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return;

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 3);		// 1000us
    if (!g_pstTimer1ms) return;

    s_pstTimer1msRed = pifTimerManager_Add(g_pstTimer1ms, TT_REPEAT);
    if (!s_pstTimer1msRed) return;
    pifTimer_AttachEvtFinish(s_pstTimer1msRed, _evtLedRedToggle, s_pstTimer1msRed);
    pifTimer_Start(s_pstTimer1msRed, 200);								// 200ms

    s_pstTimer1msYellow = pifTimerManager_Add(g_pstTimer1ms, TT_REPEAT);
    if (!s_pstTimer1msYellow) return;
    pifTimer_AttachEvtFinish(s_pstTimer1msYellow, _evtLedYellowToggle, s_pstTimer1msYellow);

    s_pstTimer1msGreen = pifTimerManager_Add(g_pstTimer1ms, TT_REPEAT);
    if (!s_pstTimer1msGreen) return;
    pifTimer_AttachEvtFinish(s_pstTimer1msGreen, _evtLedGreenToggle, s_pstTimer1msGreen);

    s_pstLedRGB = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedRGBState);
    if (!s_pstLedRGB) return;
}
