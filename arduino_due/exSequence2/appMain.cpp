#include "appMain.h"
#include "exSequence2.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifSequence.h"
#include "pifSensorSwitch.h"


PifPulse *g_pstTimer1ms = NULL;

static PifLed *s_pstLedL = NULL;
static PifLed *s_pstLedRGB = NULL;

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceRun(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceStop(PifSequence *pstOwner);

const PifSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequenceStart, 1 },
		{ _fnSequenceRun, 2 },
		{ _fnSequenceStop, PIF_SEQUENCE_PHASE_NO_IDLE }
};

static struct {
	PIF_stSensor *pstPushSwitch;
	PifSequence *pstSequence;
	BOOL bSequenceParam;
} s_stSequenceTest[SEQUENCE_COUNT] = {
		{ NULL, NULL, FALSE },
		{ NULL, NULL, FALSE }
};


static void _evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	uint8_t index = usPifId - PIF_ID_SWITCH;

	(void)pvIssuer;

	if (usLevel) {
		pifSequence_Start(s_stSequenceTest[index].pstSequence);
	}
	else {
		s_stSequenceTest[index].bSequenceParam = TRUE;
	}
	pifLog_Printf(LT_INFO, "Switch(%d): %d", usPifId, usLevel);
}

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner)
{
	uint8_t index;

	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_id - PIF_ID_SEQUENCE;
		pifLed_EachOn(s_pstLedRGB, index);
		s_stSequenceTest[index].bSequenceParam = FALSE;
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
	if (*(BOOL *)pstOwner->p_param) {
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
		pifLed_EachOff(s_pstLedRGB, index);
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
	PifComm *pstCommLog;
	int i;

	pif_Init(act_timer1us);
    pifLog_Init();

	g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);															// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;										// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;										// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;															// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    s_pstLedRGB = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, SEQUENCE_COUNT, actLedRGBState);
    if (!s_pstLedRGB) return;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
    	s_stSequenceTest[i].pstPushSwitch = pifSensorSwitch_Create(PIF_ID_SWITCH + i, 0);
		if (!s_stSequenceTest[i].pstPushSwitch) return;
	    if (!pifSensorSwitch_AttachTask(s_stSequenceTest[i].pstPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;	// 10ms
		pifSensor_AttachAction(s_stSequenceTest[i].pstPushSwitch, actPushSwitchAcquire);
		pifSensor_AttachEvtChange(s_stSequenceTest[i].pstPushSwitch, _evtPushSwitchChange, NULL);

		s_stSequenceTest[i].pstSequence = pifSequence_Create(PIF_ID_SEQUENCE + i, g_pstTimer1ms, s_astSequencePhaseList,
				&s_stSequenceTest[i].bSequenceParam);
	    if (!s_stSequenceTest[i].pstSequence) return;
	    if (!pifSequence_AttachTask(s_stSequenceTest[i].pstSequence, TM_PERIOD_MS, 10, TRUE)) return;			// 10ms
	    s_stSequenceTest[i].pstSequence->evt_error = _evtSequenceError;
    }
}
