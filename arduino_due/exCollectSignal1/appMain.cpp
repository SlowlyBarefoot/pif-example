#include "appMain.h"
#include "exCollectSignal1.h"

#include "pifCollectSignal.h"
#include "pifGpio.h"
#include "pifLed.h"
#include "pifLog.h"
#include "pifSensorSwitch.h"
#include "pifSequence.h"


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedL = NULL;
static PIF_stGpio *s_pstGpioRGB = NULL;
static PIF_stLed *s_pstLedCollect = NULL;

static PIF_enSequenceResult _fnSequenceStart(PIF_stSequence *pstOwner);
static PIF_enSequenceResult _fnSequenceRun(PIF_stSequence *pstOwner);
static PIF_enSequenceResult _fnSequenceStop(PIF_stSequence *pstOwner);

const PIF_stSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequenceStart, 1 },
		{ _fnSequenceRun, 2 },
		{ _fnSequenceStop, PIF_SEQUENCE_PHASE_NO_IDLE }
};

static BOOL bCollect = FALSE;

static struct {
	PIF_stSensor *pstPushSwitch;
	PIF_stSequence *pstSequence;
	BOOL bSequenceParam;
} s_stSequenceTest[SEQUENCE_COUNT] = {
		{ NULL, NULL, FALSE },
		{ NULL, NULL, FALSE }
};


static void _evtPushSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer)
{
	uint8_t index = usPifId - PIF_ID_SWITCH;

	(void)pvIssuer;

	if (usLevel) {
		if (s_stSequenceTest[index].pstSequence->_ucPhaseNo == PIF_SEQUENCE_PHASE_NO_IDLE) {
			pifSequence_Start(s_stSequenceTest[index].pstSequence);
		}
	}
	else {
		if (s_stSequenceTest[index].pstSequence->_ucPhaseNo != PIF_SEQUENCE_PHASE_NO_IDLE) {
			s_stSequenceTest[index].bSequenceParam = TRUE;
		}
	}
}

static void _evtPushSwitchCollectChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	if (usLevel) {
		if (!bCollect) {
			pifLed_AllOn(s_pstLedCollect);
			pifLog_Disable();
		    pifCollectSignal_Start();
			bCollect = TRUE;
		}
		else {
			pifLed_AllOff(s_pstLedCollect);
		    pifCollectSignal_Stop();
			pifLog_Enable();
			bCollect = FALSE;
		}
	}
}

static PIF_enSequenceResult _fnSequenceStart(PIF_stSequence *pstOwner)
{
	uint8_t index;

	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_usPifId - PIF_ID_SEQUENCE;
		pifGpio_WriteCell(s_pstGpioRGB, index, ON);
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
		pstOwner->unDelay1us = 1000000UL;
		return SR_enNext;
	}
	return SR_enContinue;
}

static PIF_enSequenceResult _fnSequenceStop(PIF_stSequence *pstOwner)
{
	uint8_t index;

	switch (pstOwner->ucStep) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_usPifId - PIF_ID_SEQUENCE;
		pifGpio_WriteCell(s_pstGpioRGB, index, OFF);
		return SR_enNext;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_enFinish;
	}
	return SR_enContinue;
}

void appSetup(PIF_actTimer1us actTimer1us)
{
	PIF_stComm *pstCommLog;
	PIF_stSensor *pstPushSwitchCollect;
	int i;

	pif_Init(actTimer1us);
    pifLog_Init();

    pifCollectSignal_Init("example");

	g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);														// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;									// 100%

    pstCommLog = pifComm_Init(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return;									// 1ms
	pifComm_AttachActReceiveData(pstCommLog, actLogReceiveData);
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Init(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;														// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    s_pstGpioRGB = pifGpio_Create(PIF_ID_AUTO, SEQUENCE_COUNT);
    if (!s_pstGpioRGB) return;
    pifGpio_AttachActOut(s_pstGpioRGB, actGpioRGBState);
    pifGpio_SetCsFlagAll(GpCsF_enAllBit);

    s_pstLedCollect = pifLed_Init(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedCollectState);
    if (!s_pstLedCollect) return;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
    	s_stSequenceTest[i].pstPushSwitch = pifSensorSwitch_Create(PIF_ID_SWITCH + i, 0);
		if (!s_stSequenceTest[i].pstPushSwitch) return;
	    if (!pifSensorSwitch_AttachTask(s_stSequenceTest[i].pstPushSwitch, TM_enPeriodMs, 5, TRUE)) return;	// 5ms
	    pifSensorSwitch_SetCsFlagEach(s_stSequenceTest[i].pstPushSwitch, SSCsF_enRawBit);
		pifSensor_AttachAction(s_stSequenceTest[i].pstPushSwitch, actPushSwitchAcquire);
		pifSensor_AttachEvtChange(s_stSequenceTest[i].pstPushSwitch, _evtPushSwitchChange, NULL);

		s_stSequenceTest[i].pstSequence = pifSequence_Create(PIF_ID_SEQUENCE + i, g_pstTimer1ms, s_astSequencePhaseList,
				&s_stSequenceTest[i].bSequenceParam);
	    if (!s_stSequenceTest[i].pstSequence) return;
	    if (!pifSequence_AttachTask(s_stSequenceTest[i].pstSequence, TM_enPeriodMs, 10, TRUE)) return;		// 10ms
    }
    pifSequence_SetCsFlagAll(SqCsF_enAllBit);

    pstPushSwitchCollect = pifSensorSwitch_Create(PIF_ID_AUTO, 0);
	if (!pstPushSwitchCollect) return;
    if (!pifSensorSwitch_AttachTask(pstPushSwitchCollect, TM_enPeriodMs, 5, TRUE)) return;					// 5ms
	pifSensor_AttachAction(pstPushSwitchCollect, actPushSwitchCollectAcquire);
	pifSensor_AttachEvtChange(pstPushSwitchCollect, _evtPushSwitchCollectChange, NULL);
}
