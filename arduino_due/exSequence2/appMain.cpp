#include "appMain.h"


PifLed g_led_l;
PifLed g_led_rgb;
PifTimerManager g_timer_1ms;

SequenceTest g_stSequenceTest[SEQUENCE_COUNT];

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceRun(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceStop(PifSequence *pstOwner);

const PifSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequenceStart, 1 },
		{ _fnSequenceRun, 2 },
		{ _fnSequenceStop, PIF_SEQUENCE_PHASE_NO_IDLE }
};


static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	SequenceTest* p_test = (SequenceTest*)p_issuer;

	(void)p_value;

	if (state) {
		pifSequence_Start(&p_test->stSequence);
	}
	else {
		p_test->bSequenceParam = TRUE;
	}
	pifLog_Printf(LT_INFO, "Switch(%d): %d", p_owner->_id, state);
}

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner)
{
	uint8_t index;
	SequenceTest* p_test = (SequenceTest*)pstOwner->p_param;

	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_id - PIF_ID_SEQUENCE;
		pifLed_PartOn(&g_led_rgb, 1 << index);
		p_test->bSequenceParam = FALSE;
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

static PifSequenceResult _fnSequenceRun(PifSequence *pstOwner)
{
	SequenceTest* p_test = (SequenceTest*)pstOwner->p_param;

	if (p_test->bSequenceParam) {
		pstOwner->delay1us = 1000000UL;
		return SR_NEXT;
	}
	return SR_CONTINUE;
}

static PifSequenceResult _fnSequenceStop(PifSequence *pstOwner)
{
	uint8_t index;

	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_id - PIF_ID_SEQUENCE;
		pifLed_PartOff(&g_led_rgb, 1 << index);
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

static void _evtSequenceError(PifSequence *pstOwner)
{
	(void)pstOwner;

	pifLog_Printf(LT_ERROR, "Sequence Error: %d", pif_error);
}

BOOL appSetup()
{
	int i;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
	    if (!pifSensorSwitch_AttachTaskAcquire(&g_stSequenceTest[i].stPushSwitch, TM_PERIOD_MS, 10, TRUE)) return FALSE;	// 10ms
		g_stSequenceTest[i].stPushSwitch.parent.evt_change = _evtPushSwitchChange;
		g_stSequenceTest[i].stPushSwitch.parent.p_issuer = &g_stSequenceTest[i];

	    if (!pifSequence_Init(&g_stSequenceTest[i].stSequence, PIF_ID_SEQUENCE + i, &g_timer_1ms, 10,						// 10ms
				s_astSequencePhaseList, &g_stSequenceTest[i])) return FALSE;
	    g_stSequenceTest[i].stSequence.evt_error = _evtSequenceError;

	    g_stSequenceTest[i].bSequenceParam = FALSE;
    }

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;																	// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
