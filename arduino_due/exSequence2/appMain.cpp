#include "appMain.h"
#include "exSequence2.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_sequence.h"
#include "pif_sensor_switch.h"


PifTimerManager g_timer_1ms;

static PifLed s_led_l;
static PifLed s_led_rgb;

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceRun(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceStop(PifSequence *pstOwner);

const PifSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequenceStart, 1 },
		{ _fnSequenceRun, 2 },
		{ _fnSequenceStop, PIF_SEQUENCE_PHASE_NO_IDLE }
};

typedef struct {
	PifSensorSwitch stPushSwitch;
	PifSequence stSequence;
	BOOL bSequenceParam;
} SequenceTest;

static SequenceTest s_stSequenceTest[SEQUENCE_COUNT];


static void _evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	SequenceTest* p_test = (SequenceTest*)pvIssuer;

	if (usLevel) {
		pifSequence_Start(&p_test->stSequence);
	}
	else {
		p_test->bSequenceParam = TRUE;
	}
	pifLog_Printf(LT_INFO, "Switch(%d): %d", usPifId, usLevel);
}

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner)
{
	uint8_t index;
	SequenceTest* p_test = (SequenceTest*)pstOwner->p_param;

	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_id - PIF_ID_SEQUENCE;
		pifLed_EachOn(&s_led_rgb, index);
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
		pifLed_EachOff(&s_led_rgb, index);
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

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	int i;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(6)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 4)) return;										// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;										// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;																// 500ms

    if (!pifLed_Init(&s_led_rgb, PIF_ID_AUTO, &g_timer_1ms, SEQUENCE_COUNT, actLedRGBState)) return;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
		if (!pifSensorSwitch_Init(&s_stSequenceTest[i].stPushSwitch, PIF_ID_SWITCH + i, 0)) return;
	    if (!pifSensorSwitch_AttachTask(&s_stSequenceTest[i].stPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;		// 10ms
		pifSensor_AttachAction(&s_stSequenceTest[i].stPushSwitch.parent, actPushSwitchAcquire);
		pifSensor_AttachEvtChange(&s_stSequenceTest[i].stPushSwitch.parent, _evtPushSwitchChange, &s_stSequenceTest[i]);

	    if (!pifSequence_Init(&s_stSequenceTest[i].stSequence, PIF_ID_SEQUENCE + i, &g_timer_1ms, 10,			// 10ms
				s_astSequencePhaseList, &s_stSequenceTest[i])) return;
	    s_stSequenceTest[i].stSequence.evt_error = _evtSequenceError;

	    s_stSequenceTest[i].bSequenceParam = FALSE;
    }

    pifLed_BlinkOn(&s_led_l, 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
