// Do not remove the include below
#include <MsTimer2.h>

#include "exSwitch3.h"

#include "pifLog.h"
#include "pifSwitch.h"
#include "pifPulse.h"


#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_PUSH_SWITCH			5
#define PIN_TILT_SWITCH			6

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              2
#define SWITCH_COUNT            2


static PIF_stPulse *g_pstTimer1ms = NULL;
static PIF_stSwitch *s_pstPushSwitch = NULL;
static PIF_stSwitchFilter s_stPushSwitchFilter;
static PIF_stSwitch *s_pstTiltSwitch = NULL;
static PIF_stSwitchFilter s_stTiltSwitchFilter;


static void _LogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _SwitchReceive(void *pvIssuer)
{
	(void)pvIssuer;

	pifSwitch_sigData(s_pstPushSwitch, digitalRead(PIN_PUSH_SWITCH));
	pifSwitch_sigData(s_pstTiltSwitch, digitalRead(PIN_TILT_SWITCH));
}

static void _PushSwitchChange(PIF_stSwitch *pstOwner)
{
	digitalWrite(PIN_LED_RED, pstOwner->swCurrState);
}

static void _TiltSwitchChange(PIF_stSwitch *pstOwner)
{
	digitalWrite(PIN_LED_YELLOW, pstOwner->swCurrState);
}

void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(g_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_stPulseItem *pstTimerSwitch;

	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);
	pinMode(PIN_TILT_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_LogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!g_pstTimer1ms) return;

    pstTimerSwitch = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimerSwitch) return;
    pifPulse_AttachEvtFinish(pstTimerSwitch, _SwitchReceive, NULL);

    if (!pifSwitch_Init(SWITCH_COUNT)) return;

    s_pstPushSwitch = pifSwitch_Add(1, 0);
    if (!s_pstPushSwitch) return;
    s_pstPushSwitch->bStateReverse = TRUE;
    s_pstPushSwitch->evtChange = _PushSwitchChange;
    if (!pifSwitch_AttachFilter(s_pstPushSwitch, PIF_SWITCH_FILTER_COUNT, 5, &s_stPushSwitchFilter)) return;

    s_pstTiltSwitch = pifSwitch_Add(2, 0);
	if (!s_pstTiltSwitch) return;
	s_pstTiltSwitch->evtChange = _TiltSwitchChange;
    if (!pifSwitch_AttachFilter(s_pstTiltSwitch, PIF_SWITCH_FILTER_CONTINUE, 5, &s_stTiltSwitchFilter)) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;
    if (!pifTask_AddRatio(3, pifSwitch_taskAll, NULL)) return;

    pifPulse_StartItem(pstTimerSwitch, 20);		// 20 * 1ms = 20ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
