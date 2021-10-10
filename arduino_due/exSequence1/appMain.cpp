#include "appMain.h"
#include "exSequence1.h"

#include "pifLog.h"
#include "pifSequence.h"


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stSequence *s_pstSequence = NULL;

static PIF_enSequenceResult _fnSequence1(PIF_stSequence *pstOwner);
static PIF_enSequenceResult _fnSequence2(PIF_stSequence *pstOwner);
static PIF_enSequenceResult _fnSequence3(PIF_stSequence *pstOwner);

const PIF_stSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequence1, 1 },
		{ _fnSequence2, 2 },
		{ _fnSequence3, PIF_SEQUENCE_PHASE_NO_IDLE }
};


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
		pstOwner->unDelay1us = 100000UL;
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
		pifLog_Printf(LT_enInfo, "Sequence3: Complete");
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

	pifLog_Printf(LT_enError, "Sequence Error: %d", pif_error);
}

static uint16_t _taskSequence(PifTask *pstTask)
{
	(void)pstTask;

	switch (s_pstSequence->_ucPhaseNo) {
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
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	PIF_stComm *pstCommLog;

	pif_Init(act_timer1us);
    pifLog_Init();

	g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;			// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstSequence = pifSequence_Create(PIF_ID_AUTO, g_pstTimer1ms, s_astSequencePhaseList, NULL);
    if (!s_pstSequence) return;
    if (!pifSequence_AttachTask(s_pstSequence, TM_PERIOD_MS, 10, TRUE)) return;	// 10ms
    s_pstSequence->evtError = _evtSequenceError;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskSequence, NULL, TRUE)) return;	// 500ms

    pifSequence_Start(s_pstSequence);
}
