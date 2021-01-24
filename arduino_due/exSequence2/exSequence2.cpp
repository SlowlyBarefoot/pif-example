// Do not remove the include below
#include "exSequence2.h"

#include "pifLog.h"
#include "pifPulse.h"
#include "pifSequence.h"
#include "pifSwitch.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define SEQUENCE_COUNT          2
#define SWITCH_COUNT            2
#define TASK_COUNT              3

#define PIF_ID_SWITCH			0x100
#define PIF_ID_2_INDEX(id)		((id) - PIF_ID_SWITCH)


static PIF_stPulse *s_pstTimer1ms = NULL;

static PIF_enSequenceResult _fnSequenceStart(PIF_stSequence *pstOwner);
static PIF_enSequenceResult _fnSequenceRun(PIF_stSequence *pstOwner);
static PIF_enSequenceResult _fnSequenceStop(PIF_stSequence *pstOwner);

const PIF_stSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequenceStart, 1 },
		{ _fnSequenceRun, 2 },
		{ _fnSequenceStop, PIF_SEQUENCE_PHASE_NO_IDLE }
};

static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
	PIF_stSwitch *pstPushSwitch;
	PIF_stSequence *pstSequence;
	BOOL bSequenceParam;
} s_stSequenceTest[SEQUENCE_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED, NULL, NULL, FALSE },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW, NULL, NULL, FALSE }
};


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static SWITCH _actPushSwitchAcquire(PIF_usId usPifId)
{
	return digitalRead(s_stSequenceTest[PIF_ID_2_INDEX(usPifId)].ucPinSwitch);
}

static void _evtPushSwitchChange(PIF_usId usPifId, SWITCH swState, void *pvIssuer)
{
	uint8_t index = PIF_ID_2_INDEX(usPifId);

	(void)pvIssuer;

	if (swState) {
		pifSequence_Start(s_stSequenceTest[index].pstSequence);
	}
	else {
		s_stSequenceTest[index].bSequenceParam = TRUE;
	}
	pifLog_Printf(LT_enInfo, "Switch(%d): %d", usPifId, swState);
}

static PIF_enSequenceResult _fnSequenceStart(PIF_stSequence *pstOwner)
{
	uint8_t index;

	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
		index = PIF_ID_2_INDEX(pstOwner->_usPifId);
		digitalWrite(s_stSequenceTest[index].ucPinLed, ON);
		s_stSequenceTest[index].bSequenceParam = FALSE;
		return SR_enNext;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_enFinish;
	}
	return SR_enContinue;
}

static PIF_enSequenceResult _fnSequenceRun(PIF_stSequence *pstOwner)
{
	if (*(BOOL *)pstOwner->pvParam) {
		pstOwner->usDelay = 1000;
		return SR_enNext;
	}
	return SR_enContinue;
}

static PIF_enSequenceResult _fnSequenceStop(PIF_stSequence *pstOwner)
{
	uint8_t index;

	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
		index = PIF_ID_2_INDEX(pstOwner->_usPifId);
		digitalWrite(s_stSequenceTest[index].ucPinLed, OFF);
		return SR_enNext;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_enFinish;
	}
	return SR_enContinue;
}

static void _evtSequenceError(PIF_stSequence *pstOwner)
{
	(void)pstOwner;

	pifLog_Printf(LT_enError, "Sequence Error: %d", pif_enError);
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(s_pstTimer1ms);
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	int i;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	Serial.begin(115200); //Doesn't matter speed

	pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

	if (!pifPulse_Init(PULSE_COUNT)) return;
	s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;

    if (!pifSwitch_Init(SWITCH_COUNT)) return;
    if (!pifSequence_Init(s_pstTimer1ms, SEQUENCE_COUNT)) return;

    for (i = 0; i < SWITCH_COUNT; i++) {
    	s_stSequenceTest[i].pstPushSwitch = pifSwitch_Add(PIF_ID_SWITCH + i, 0);
		if (!s_stSequenceTest[i].pstPushSwitch) return;
		s_stSequenceTest[i].pstPushSwitch->bStateReverse = TRUE;
		pifSwitch_AttachAction(s_stSequenceTest[i].pstPushSwitch, _actPushSwitchAcquire);
		pifSwitch_AttachEvtChange(s_stSequenceTest[i].pstPushSwitch, _evtPushSwitchChange, NULL);

		s_stSequenceTest[i].pstSequence = pifSequence_Add(PIF_ID_SWITCH + i, s_astSequencePhaseList,
				&s_stSequenceTest[i].bSequenceParam);
	    if (!s_stSequenceTest[i].pstSequence) return;
	    s_stSequenceTest[i].pstSequence->evtError = _evtSequenceError;
    }

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;			// 100%
    if (!pifTask_AddRatio(3, pifSwitch_taskAll, NULL)) return;			// 3%
    if (!pifTask_AddPeriodMs(10, pifSequence_taskAll, NULL)) return;	// 10ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
