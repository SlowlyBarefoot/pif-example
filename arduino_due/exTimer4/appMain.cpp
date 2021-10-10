#include "appMain.h"
#include "exTimer4.h"

#include "pifLed.h"


PifPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedRGB = NULL;
static PifPulseItem *s_pstTimer1msRed = NULL;
static PifPulseItem *s_pstTimer1msYellow = NULL;
static PifPulseItem *s_pstTimer1msGreen = NULL;


static void _evtLedRedToggle(void *pvIssuer)
{
	static int count = 0;

	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 0);

	count++;
	if (count >= 10) {
		count = 0;
		pifPulse_StopItem((PifPulseItem *)pvIssuer);
		pifPulse_StartItem(s_pstTimer1msYellow, 500);	// 500 * 1ms = 0.5sec
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
		pifPulse_StopItem((PifPulseItem *)pvIssuer);
		pifPulse_StartItem(s_pstTimer1msGreen, 1000);	// 1000 * 1ms = 1sec
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
		pifPulse_StopItem((PifPulseItem *)pvIssuer);
		pifPulse_StartItem(s_pstTimer1msRed, 200);		// 200 * 1ms = 0.2sec
	}
}

void appSetup()
{
	pif_Init(NULL);

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;	// 100%

    s_pstTimer1msRed = pifPulse_AddItem(g_pstTimer1ms, PT_REPEAT);
    if (!s_pstTimer1msRed) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msRed, _evtLedRedToggle, s_pstTimer1msRed);
    pifPulse_StartItem(s_pstTimer1msRed, 200);								// 200ms

    s_pstTimer1msYellow = pifPulse_AddItem(g_pstTimer1ms, PT_REPEAT);
    if (!s_pstTimer1msYellow) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msYellow, _evtLedYellowToggle, s_pstTimer1msYellow);

    s_pstTimer1msGreen = pifPulse_AddItem(g_pstTimer1ms, PT_REPEAT);
    if (!s_pstTimer1msGreen) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msGreen, _evtLedGreenToggle, s_pstTimer1msGreen);

    s_pstLedRGB = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedRGBState);
    if (!s_pstLedRGB) return;
}
