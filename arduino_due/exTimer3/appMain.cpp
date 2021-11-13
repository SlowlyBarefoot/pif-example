#include "appMain.h"
#include "exTimer3.h"

#include "pif_led.h"


PifPulse *g_pstTimer1ms = NULL;

static PifLed *s_pstLedRGB = NULL;
static PifPulseItem *s_pstTimer1msRed = NULL;
static PifPulseItem *s_pstTimer1msYellow = NULL;
static PifPulseItem *s_pstTimer1msGreen = NULL;


static void _evtLedRedToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 0);

	pifPulse_StartItem(s_pstTimer1msYellow, 500);	// 500 * 1ms = 0.5sec
}

static void _evtLedYellowToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 1);

	pifPulse_StartItem(s_pstTimer1msGreen, 1000);	// 1000 * 1ms = 1sec
}

static void _evtLedGreenToggle(void *pvIssuer)
{
	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 2);

	pifPulse_StartItem(s_pstTimer1msRed, 200);		// 200 * 1ms = 0.2sec
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return;

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000, 3);					// 1000us
    if (!g_pstTimer1ms) return;

    s_pstTimer1msRed = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
    if (!s_pstTimer1msRed) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msRed, _evtLedRedToggle, NULL);
    pifPulse_StartItem(s_pstTimer1msRed, 200);								// 200ms

    s_pstTimer1msYellow = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
    if (!s_pstTimer1msYellow) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msYellow, _evtLedYellowToggle, NULL);

    s_pstTimer1msGreen = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
    if (!s_pstTimer1msGreen) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msGreen, _evtLedGreenToggle, NULL);

    s_pstLedRGB = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedRGBState);
    if (!s_pstLedRGB) return;
}
