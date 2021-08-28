#include "appMain.h"
#include "exTimer4.h"

#include "pifLed.h"


#define LED_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              3


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedRGB = NULL;
static PIF_stPulseItem *s_pstTimer1msRed = NULL;
static PIF_stPulseItem *s_pstTimer1msYellow = NULL;
static PIF_stPulseItem *s_pstTimer1msGreen = NULL;


static void _evtLedRedToggle(void *pvIssuer)
{
	static int count = 0;

	(void)pvIssuer;

	pifLed_EachToggle(s_pstLedRGB, 0);

	count++;
	if (count >= 10) {
		count = 0;
		pifPulse_StopItem((PIF_stPulseItem *)pvIssuer);
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
		pifPulse_StopItem((PIF_stPulseItem *)pvIssuer);
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
		pifPulse_StopItem((PIF_stPulseItem *)pvIssuer);
		pifPulse_StartItem(s_pstTimer1msRed, 200);		// 200 * 1ms = 0.2sec
	}
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    s_pstTimer1msRed = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!s_pstTimer1msRed) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msRed, _evtLedRedToggle, s_pstTimer1msRed);
    pifPulse_StartItem(s_pstTimer1msRed, 200);								// 200ms

    s_pstTimer1msYellow = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!s_pstTimer1msYellow) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msYellow, _evtLedYellowToggle, s_pstTimer1msYellow);

    s_pstTimer1msGreen = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!s_pstTimer1msGreen) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msGreen, _evtLedGreenToggle, s_pstTimer1msGreen);

    s_pstLedRGB = pifLed_Add(PIF_ID_AUTO, 3, actLedRGBState);
    if (!s_pstLedRGB) return;

    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
}
