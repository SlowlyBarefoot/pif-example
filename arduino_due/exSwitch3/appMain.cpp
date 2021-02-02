#include "appMain.h"
#include "exSwitch3.h"

#include "pifLed.h"
#include "pifLog.h"


#define LED_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              2
#define SWITCH_COUNT            2


PIF_stSwitch *g_pstPushSwitch = NULL;
PIF_stSwitch *g_pstTiltSwitch = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLed = NULL;
static PIF_stSwitchFilter s_stPushSwitchFilter;
static PIF_stSwitchFilter s_stTiltSwitchFilter;


static void _evtPushSwitchChange(PIF_usId usPifId, SWITCH swState, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	pifLed_Change(s_pstLed, 1, swState);
}

static void _evtTiltSwitchChange(PIF_usId usPifId, SWITCH swState, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	pifLed_Change(s_pstLed, 2, swState);
}

void appSetup()
{
	PIF_stPulseItem *pstTimerSwitch;

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    s_pstLed = pifLed_Add(PIF_ID_AUTO, 3, actLedState);
    if (!s_pstLed) return;
    if (!pifLed_AttachBlink(s_pstLed, 500)) return;							// 500ms
    pifLed_BlinkOn(s_pstLed, 0);

    pstTimerSwitch = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimerSwitch) return;
    pifPulse_AttachEvtFinish(pstTimerSwitch, evtSwitchAcquire, NULL);

    if (!pifSwitch_Init(SWITCH_COUNT)) return;

    g_pstPushSwitch = pifSwitch_Add(PIF_ID_AUTO, 0);
    if (!g_pstPushSwitch) return;
    g_pstPushSwitch->bStateReverse = TRUE;
    pifSwitch_AttachEvtChange(g_pstPushSwitch, _evtPushSwitchChange, NULL);
    if (!pifSwitch_AttachFilter(g_pstPushSwitch, PIF_SWITCH_FILTER_COUNT, 5, &s_stPushSwitchFilter)) return;

    g_pstTiltSwitch = pifSwitch_Add(PIF_ID_AUTO, 0);
	if (!g_pstTiltSwitch) return;
	pifSwitch_AttachEvtChange(g_pstTiltSwitch, _evtTiltSwitchChange, NULL);
    if (!pifSwitch_AttachFilter(g_pstTiltSwitch, PIF_SWITCH_FILTER_CONTINUE, 5, &s_stTiltSwitchFilter)) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddRatio(3, pifSwitch_taskAll, NULL)) return;		// 3%

    pifPulse_StartItem(pstTimerSwitch, 20);							// 20ms
}
