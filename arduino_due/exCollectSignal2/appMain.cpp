#include "appMain.h"


PifLed g_led_l;
PifGpio g_gpio_rgb;
PifLed g_led_collect;
PifSensorSwitch g_push_switch_collect;
PifTimerManager g_timer_1ms;

TestStruct g_test[SEQUENCE_COUNT];

static PifSequenceResult _fnSequenceStart(PifSequence* p_owner);
static PifSequenceResult _fnSequenceRun(PifSequence* p_owner);
static PifSequenceResult _fnSequenceStop(PifSequence* p_owner);

const PifSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequenceStart, 1 },
		{ _fnSequenceRun, 2 },
		{ _fnSequenceStop, PIF_SEQUENCE_PHASE_NO_IDLE }
};

static BOOL bCollect = FALSE;


static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	TestStruct* p_test = (TestStruct*)p_issuer;

	(void)p_owner;
	(void)p_value;

	if (state) {
		if (p_test->stSequence._phase_no == PIF_SEQUENCE_PHASE_NO_IDLE) {
			pifSequence_Start(&p_test->stSequence);
		}
	}
	else {
		if (p_test->stSequence._phase_no != PIF_SEQUENCE_PHASE_NO_IDLE) {
			p_test->bSequenceParam = TRUE;
		}
	}
}

static void _evtPushSwitchCollectChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	if (state) {
		if (!bCollect) {
			pifLed_AllOn(&g_led_collect);
		    pifCollectSignal_Start();
			bCollect = TRUE;
		}
		else {
			pifLed_AllOff(&g_led_collect);
		    pifCollectSignal_Stop();
		    pifCollectSignal_PrintLog();
			bCollect = FALSE;
		}
	}
}

static PifSequenceResult _fnSequenceStart(PifSequence* p_owner)
{
	uint8_t index;
	TestStruct* p_test = (TestStruct*)p_owner->p_param;

	switch (p_owner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = p_owner->_id - PIF_ID_SEQUENCE;
		pifGpio_WriteCell(&g_gpio_rgb, index, ON);
		p_test->bSequenceParam = FALSE;
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

static PifSequenceResult _fnSequenceRun(PifSequence* p_owner)
{
	TestStruct* p_test = (TestStruct*)p_owner->p_param;

	if (p_test->bSequenceParam) {
		p_owner->delay1us = 1000000UL;
		return SR_NEXT;
	}
	return SR_CONTINUE;
}

static PifSequenceResult _fnSequenceStop(PifSequence* p_owner)
{
	uint8_t index;

	switch (p_owner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = p_owner->_id - PIF_ID_SEQUENCE;
		pifGpio_WriteCell(&g_gpio_rgb, index, OFF);
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

BOOL appSetup()
{
	int i;

    pifGpioColSig_SetFlag(GP_CSF_ALL_BIT);

    for (i = 0; i < SEQUENCE_COUNT; i++) {
	    if (!pifSensorSwitch_AttachTaskAcquire(&g_test[i].stPushSwitch, TM_PERIOD_MS, 5, TRUE)) return FALSE;	// 5ms
	    pifSensorSwitch_SetCsFlag(&g_test[i].stPushSwitch, SS_CSF_FILTER_BIT);
		g_test[i].stPushSwitch.parent.evt_change = _evtPushSwitchChange;
		g_test[i].stPushSwitch.parent.p_issuer = &g_test[i];

	    if (!pifSequence_Init(&g_test[i].stSequence, PIF_ID_SEQUENCE + i, &g_timer_1ms, 10,						// 10ms
				s_astSequencePhaseList,	&g_test[i])) return FALSE;
    }

    pifSequenceColSig_SetFlag(SQ_CSF_ALL_BIT);

    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch_collect, TM_PERIOD_MS, 5, TRUE)) return FALSE;		// 5ms
	g_push_switch_collect.parent.evt_change = _evtPushSwitchCollectChange;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;														// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
