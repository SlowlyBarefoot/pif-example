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

const PIF_stSequenceItem s_astSequenceItemList[] = {
		{ _fnSequence1, 	0, 		1, 							PIF_SEQUENCE_ITEM_NO_IDLE },
		{ _fnSequence2, 	100,	2, 							PIF_SEQUENCE_ITEM_NO_IDLE },
		{ _fnSequence3, 	0, 		PIF_SEQUENCE_ITEM_NO_IDLE,	PIF_SEQUENCE_ITEM_NO_IDLE }
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

	case 2:
		pifLog_Printf(LT_enInfo, "Sequence1: Next");
		return SR_enNext;
	}
	return SR_enContinue;
}

static PIF_enSequenceResult _fnSequence2(PIF_stSequence *pstOwner)
{
	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
		pstOwner->ucStep = 1;
		pifLog_Printf(LT_enInfo, "Sequence2: %d", pstOwner->ucStep);
		break;

	case 2:
		pstOwner->ucStep = 3;
		pifLog_Printf(LT_enInfo, "Sequence2: %d", pstOwner->ucStep);
		break;

	case 4:
		pifLog_Printf(LT_enInfo, "Sequence2: Next");
		return SR_enNext;
	}
	return SR_enContinue;
}

static PIF_enSequenceResult _fnSequence3(PIF_stSequence *pstOwner)
{
	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
		pstOwner->ucStep = 1;
		pifLog_Printf(LT_enInfo, "Sequence3: %d", pstOwner->ucStep);
		break;

	case 2:
		pstOwner->ucStep = 3;
		pifLog_Printf(LT_enInfo, "Sequence3: %d", pstOwner->ucStep);
		break;

	case 4:
		pstOwner->ucStep = 5;
		pifLog_Printf(LT_enInfo, "Sequence3: %d", pstOwner->ucStep);
		break;

	case 6:
		pifLog_Printf(LT_enInfo, "Sequence3: Next");
		return SR_enNext;
	}
	return SR_enContinue;
}

static void _taskSequence(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;

	switch (s_pstSequence->ucItemNo) {
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
	PIF_unDeviceCode unDeviceCode = 1;

	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed

	pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

	if (!pifPulse_Init(PULSE_COUNT)) return;
	s_pstTimer1ms = pifPulse_Add(unDeviceCode++, PULSE_ITEM_COUNT);
    if (!s_pstTimer1ms) return;

    if (!pifSequence_Init(s_pstTimer1ms, SEQUENCE_COUNT)) return;
    s_pstSequence = pifSequence_Add(1, s_astSequenceItemList, NULL);
    if (!s_pstSequence) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriod(10, pifSequence_taskAll, NULL)) return;	// 10ms

    if (!pifTask_AddPeriod(500, _taskSequence, NULL)) return;		// 500ms

    pifSequence_Start(s_pstSequence);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
