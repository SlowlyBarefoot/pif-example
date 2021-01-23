// Do not remove the include below
#include "exSequence1.h"

#include "pifLog.h"
#include "pifPulse.h"
#include "pifSequence.h"


#define PIN_LED_L				13

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define SEQUENCE_COUNT          1
#define TASK_COUNT              3


static PIF_stPulse *s_pstTimer1ms = NULL;

static PIF_enSequenceResult _fnSequence1(PIF_stSequence *pstOwner);
static PIF_enSequenceResult _fnSequence2(PIF_stSequence *pstOwner);
static PIF_enSequenceResult _fnSequence3(PIF_stSequence *pstOwner);

const PIF_stSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequence1, 1 },
		{ _fnSequence2, 2 },
		{ _fnSequence3, PIF_SEQUENCE_PHASE_NO_IDLE }
};
static PIF_stSequence *s_pstSequence = NULL;


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static PIF_enSequenceResult _fnSequence1(PIF_stSequence *pstOwner)
{
	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
		pstOwner->ucStep = 1;
		pifLog_Printf(LT_enInfo, "Sequence1: %d", pstOwner->ucStep);
		break;

	case 1:
		break;

	case 2:
		return SR_enNext;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_enFinish;
	}
	return SR_enContinue;
}

static PIF_enSequenceResult _fnSequence2(PIF_stSequence *pstOwner)
{
	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
	case 2:
		pstOwner->ucStep++;
		pifLog_Printf(LT_enInfo, "Sequence2: %d", pstOwner->ucStep);
		break;

	case 1:
	case 3:
		break;

	case 4:
		// 다음 Phase를 처리하기 전 일정 시간 지연이 필요한 경우 설정함.
		// Set if a delay is required before processing the next Phase.
		pstOwner->usDelay = 100;
		return SR_enNext;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_enFinish;
	}
	return SR_enContinue;
}

static PIF_enSequenceResult _fnSequence3(PIF_stSequence *pstOwner)
{
	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
		// 이 Phase의 처리 시간 제한을 설정함.
		// Set processing time limit for this Phase, for this Phase.
		pifSequence_SetTimeout(pstOwner, 1000);
		pstOwner->ucStep++;
		pifLog_Printf(LT_enInfo, "Sequence3: %d", pstOwner->ucStep);
		break;

	case 2:
	case 4:
		pstOwner->ucStep++;
		pifLog_Printf(LT_enInfo, "Sequence3: %d", pstOwner->ucStep);
		break;

	case 1:
	case 3:
	case 5:
		break;

	case 6:
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

static void _taskSequence(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;

	switch (s_pstSequence->ucPhaseNo) {
	case 0:
		switch (s_pstSequence->ucStep) {
		case 1:
			s_pstSequence->ucStep++;
			pifLog_Printf(LT_enInfo, "Sequence: %d", s_pstSequence->ucStep);
			break;
		}
		break;

	case 1:
		switch (s_pstSequence->ucStep) {
		case 1:
		case 3:
			s_pstSequence->ucStep++;
			pifLog_Printf(LT_enInfo, "Sequence: %d", s_pstSequence->ucStep);
			break;
		}
		break;

	case 2:
		switch (s_pstSequence->ucStep) {
		case 1:
		case 3:
		case 5:
			s_pstSequence->ucStep++;
			pifLog_Printf(LT_enInfo, "Sequence: %d", s_pstSequence->ucStep);
			break;
		}
		break;
	}
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
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed

	pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

	if (!pifPulse_Init(PULSE_COUNT)) return;
	s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;

    if (!pifSequence_Init(s_pstTimer1ms, SEQUENCE_COUNT)) return;
    s_pstSequence = pifSequence_Add(1, s_astSequencePhaseList, NULL);
    if (!s_pstSequence) return;
    pifSequence_AttachEvent(s_pstSequence, _evtSequenceError);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;			// 100%
    if (!pifTask_AddPeriodMs(10, pifSequence_taskAll, NULL)) return;	// 10ms

    if (!pifTask_AddPeriodMs(500, _taskSequence, NULL)) return;			// 500ms

    pifSequence_Start(s_pstSequence);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
