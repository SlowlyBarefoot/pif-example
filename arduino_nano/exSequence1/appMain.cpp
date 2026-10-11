#include "appMain.h"

#include "core/pif_sequence.h"


static int s_step = 0;
static PifSequence s_sequence;

static void _fnSequence1(PifSequence *p_owner);
static void _fnSequence2(PifSequence *p_owner);
static void _fnSequence3(PifSequence *p_owner);
static void _fnSequenceTimeout(PifSequence *p_owner);


static void _fnSequence1(PifSequence *p_owner)
{
	int *p_step = (int *)p_owner->p_param;

	(*p_step)++;
	pifLog_Printf(LT_INFO, "Sequence1: %d", *p_step);
	pifSequence_Wait(p_owner, _fnSequence2, 1000, _fnSequenceTimeout);		// 1000ms
}

static void _fnSequence2(PifSequence *p_owner)
{
	int *p_step = (int *)p_owner->p_param;

	(*p_step)++;
	pifLog_Printf(LT_INFO, "Sequence2: %d", *p_step);
	if (*p_step < 5) {
		pifSequence_Delay(p_owner, _fnSequence2, *p_step * 100);			// 100ms * step
	}
	else {
		pifSequence_Delay(p_owner, _fnSequence3, 500);						// 500ms
	}
}

static void _fnSequence3(PifSequence *p_owner)
{
	int *p_step = (int *)p_owner->p_param;

	(*p_step)++;
	pifLog_Printf(LT_INFO, "Sequence3: %d", *p_step);
}

static void _fnSequenceTimeout(PifSequence *p_owner)
{
	(void)p_owner;

	pifLog_Printf(LT_ERROR, "Sequence Timeout");
}

static uint32_t _taskSequence(PifTask *p_task)
{
	PifSequence *p_owner = (PifSequence *)p_task->_p_client;
	int *p_step = (int *)p_owner->p_param;

	switch (*p_step) {
	case 1:
		pifSequence_Signal(p_owner);
		break;
	}
	return 0;
}

BOOL appSetup()
{
    if (!pifSequence_Init(&s_sequence, PIF_ID_AUTO, &s_step)) return FALSE;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, _taskSequence, &s_sequence, TRUE)) return FALSE;	// 500ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            exSequence1           ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);

    pifSequence_Start(&s_sequence, _fnSequence1);
    return TRUE;
}
