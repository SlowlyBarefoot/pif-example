#include "appMain.h"
#include "exCollectSignal2.h"

#include "pif_collect_signal.h"
#include "pif_gpio.h"
#include "pif_led.h"
#include "pif_log.h"
#include "pif_sensor_switch.h"
#include "pif_sequence.h"


PifPulse *g_pstTimer1ms = NULL;

static PifLed *s_pstLedL = NULL;
static PifGpio *s_pstGpioRGB = NULL;
static PifLed *s_pstLedCollect = NULL;

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceRun(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceStop(PifSequence *pstOwner);

const PifSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequenceStart, 1 },
		{ _fnSequenceRun, 2 },
		{ _fnSequenceStop, PIF_SEQUENCE_PHASE_NO_IDLE }
};

static BOOL bCollect = FALSE;

static struct {
	PifSensor *pstPushSwitch;
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
		if (s_stSequenceTest[index].pstSequence->_phase_no == PIF_SEQUENCE_PHASE_NO_IDLE) {
			pifSequence_Start(s_stSequenceTest[index].pstSequence);
		}
	}
	else {
		if (s_stSequenceTest[index].pstSequence->_phase_no != PIF_SEQUENCE_PHASE_NO_IDLE) {
			s_stSequenceTest[index].bSequenceParam = TRUE;
		}
	}
}

static void _evtPushSwitchCollectChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	if (usLevel) {
		if (!bCollect) {
			pifLed_AllOn(s_pstLedCollect);
		    pifCollectSignal_Start();
			bCollect = TRUE;
		}
		else {
			pifLed_AllOff(s_pstLedCollect);
		    pifCollectSignal_Stop();
		    pifCollectSignal_PrintLog();
			bCollect = FALSE;
		}
	}
}

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner)
{
	uint8_t index;

	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_id - PIF_ID_SEQUENCE;
		pifGpio_WriteCell(s_pstGpioRGB, index, ON);
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
		pifGpio_WriteCell(s_pstGpioRGB, index, OFF);
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

void appSetup(PifActTimer1us act_timer1us)
{
	PifComm *pstCommLog;
	PifSensor *pstPushSwitchCollect;
	int i;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(8)) return;

    pifLog_Init();

	g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000, 4);													// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;										// 1ms
	pstCommLog->act_receive_data = actLogReceiveData;
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

	if (!pifCollectSignal_InitHeap("example", 0x1000)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;														// 500ms

    s_pstGpioRGB = pifGpio_Create(PIF_ID_AUTO, SEQUENCE_COUNT);
    if (!s_pstGpioRGB) return;
    pifGpio_AttachActOut(s_pstGpioRGB, actGpioRGBState);
    pifGpio_SetCsFlagAll(GP_CSF_ALL_BIT);

    s_pstLedCollect = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedCollectState);
    if (!s_pstLedCollect) return;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
    	s_stSequenceTest[i].pstPushSwitch = pifSensorSwitch_Create(PIF_ID_SWITCH + i, 0);
		if (!s_stSequenceTest[i].pstPushSwitch) return;
	    if (!pifSensorSwitch_AttachTask(s_stSequenceTest[i].pstPushSwitch, TM_PERIOD_MS, 5, TRUE)) return;	// 5ms
	    pifSensorSwitch_SetCsFlagEach(s_stSequenceTest[i].pstPushSwitch, SS_CSF_FILTER_BIT);
		pifSensor_AttachAction(s_stSequenceTest[i].pstPushSwitch, actPushSwitchAcquire);
		pifSensor_AttachEvtChange(s_stSequenceTest[i].pstPushSwitch, _evtPushSwitchChange, NULL);

		s_stSequenceTest[i].pstSequence = pifSequence_Create(PIF_ID_SEQUENCE + i, g_pstTimer1ms, 10,
				s_astSequencePhaseList,	&s_stSequenceTest[i].bSequenceParam);								// 10ms
	    if (!s_stSequenceTest[i].pstSequence) return;
    }
    pifSequence_SetCsFlagAll(SQ_CSF_ALL_BIT);

    pstPushSwitchCollect = pifSensorSwitch_Create(PIF_ID_AUTO, 0);
	if (!pstPushSwitchCollect) return;
    if (!pifSensorSwitch_AttachTask(pstPushSwitchCollect, TM_PERIOD_MS, 5, TRUE)) return;					// 5ms
	pifSensor_AttachAction(pstPushSwitchCollect, actPushSwitchCollectAcquire);
	pifSensor_AttachEvtChange(pstPushSwitchCollect, _evtPushSwitchCollectChange, NULL);

    pifLed_BlinkOn(s_pstLedL, 0);

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifPulse_Count(g_pstTimer1ms));
}
