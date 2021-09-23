#include "appMain.h"
#include "exTimer3.h"

#include "pifLed.h"


#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              3


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedRGB = NULL;
static PIF_stPulseItem *s_pstTimer1msRed = NULL;
static PIF_stPulseItem *s_pstTimer1msYellow = NULL;
static PIF_stPulseItem *s_pstTimer1msGreen = NULL;


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

    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Init(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;	// 100%

    s_pstTimer1msRed = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
    if (!s_pstTimer1msRed) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msRed, _evtLedRedToggle, NULL);
    pifPulse_StartItem(s_pstTimer1msRed, 200);								// 200ms

    s_pstTimer1msYellow = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
    if (!s_pstTimer1msYellow) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msYellow, _evtLedYellowToggle, NULL);

    s_pstTimer1msGreen = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
    if (!s_pstTimer1msGreen) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msGreen, _evtLedGreenToggle, NULL);

    s_pstLedRGB = pifLed_Init(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedRGBState);
    if (!s_pstLedRGB) return;
}
