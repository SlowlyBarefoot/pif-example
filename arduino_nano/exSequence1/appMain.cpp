#include "appMain.h"
#include "exSequence1.h"

#include "pif_log.h"
#include "pif_sequence.h"


PifTimerManager g_timer_1ms;

static PifSequence s_sequence;

static PifSequenceResult _fnSequence1(PifSequence *pstOwner);
static PifSequenceResult _fnSequence2(PifSequence *pstOwner);
static PifSequenceResult _fnSequence3(PifSequence *pstOwner);

const PifSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequence1, 1 },
		{ _fnSequence2, 2 },
		{ _fnSequence3, PIF_SEQUENCE_PHASE_NO_IDLE }
};


static PifSequenceResult _fnSequence1(PifSequence *pstOwner)
{
	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		pstOwner->step = 1;
		pifLog_Printf(LT_INFO, "Sequence1: %d", pstOwner->step);
		break;

	case 1:
		break;

	case 2:
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

static PifSequenceResult _fnSequence2(PifSequence *pstOwner)
{
	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
	case 2:
		pstOwner->step++;
		pifLog_Printf(LT_INFO, "Sequence2: %d", pstOwner->step);
		break;

	case 1:
	case 3:
		break;

	case 4:
		// 다음 Phase를 처리하기 전 일정 시간 지연이 필요한 경우 설정함.
		// Set if a delay is required before processing the next Phase.
		pstOwner->delay1us = 100000UL;
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

static PifSequenceResult _fnSequence3(PifSequence *pstOwner)
{
	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		// 이 Phase의 처리 시간 제한을 설정함.
		// Set processing time limit for this Phase, for this Phase.
		pifSequence_SetTimeout(pstOwner, 1000);
		pstOwner->step++;
		pifLog_Printf(LT_INFO, "Sequence3: %d", pstOwner->step);
		break;

	case 2:
	case 4:
		pstOwner->step++;
		pifLog_Printf(LT_INFO, "Sequence3: %d", pstOwner->step);
		break;

	case 1:
	case 3:
	case 5:
		break;

	case 6:
		pifLog_Printf(LT_INFO, "Sequence3: Complete");
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

static uint16_t _taskSequence(PifTask *pstTask)
{
	(void)pstTask;

	switch (s_sequence._phase_no) {
	case 0:
		switch (s_sequence.step) {
		case 1:
			s_sequence.step++;
			pifLog_Printf(LT_INFO, "Sequence: %d", s_sequence.step);
			break;
		}
		break;

	case 1:
		switch (s_sequence.step) {
		case 1:
		case 3:
			s_sequence.step++;
			pifLog_Printf(LT_INFO, "Sequence: %d", s_sequence.step);
			break;
		}
		break;

	case 2:
		switch (s_sequence.step) {
		case 1:
		case 3:
		case 5:
			s_sequence.step++;
			pifLog_Printf(LT_INFO, "Sequence: %d", s_sequence.step);
			break;
		}
		break;
	}
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifSequence_Init(&s_sequence, PIF_ID_AUTO, &g_timer_1ms, 10,
    		s_astSequencePhaseList, NULL)) return;									// 10ms
    s_sequence.evt_error = _evtSequenceError;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskSequence, NULL, TRUE)) return;	// 500ms

    pifSequence_Start(&s_sequence);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
