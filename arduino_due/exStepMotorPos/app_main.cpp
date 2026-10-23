#include "app_port.h"


PifLed g_led_l;
PifStepMotorPos g_motor;
PifSensorSwitch g_switch[SWITCH_COUNT];
PifTimerManager g_timer_1ms;
PifTimerManager g_timer_200us;


static int CmdStepMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "mt", CmdStepMotorTest, "Motor Test", NULL },

	{ NULL, NULL, NULL, NULL }
};

#define STEP_MOTOR_STAGE_COUNT	5

const PifStepMotorPosStage s_stStepMotorStages[STEP_MOTOR_STAGE_COUNT] = {
		{
				MM_D_CCW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &g_switch[0].parent,
				0, 0,
				50, 0,
				0, 0, 100,
				0
		},
		{
				MM_D_CW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &g_switch[2].parent,
				0, 0,
				50, 0,
				0, 0, 100,
				0
		},
		{
				MM_D_CW | MM_PC_YES,
				NULL, NULL, NULL,
				200, 50,
				800, 750,
				200, 50, 100,
				1000
		},
		{
				MM_D_CCW | MM_PC_YES,
				NULL, NULL, NULL,
				200, 50,
				800, 750,
				200, 50, 100,
				1000
		},
		{
				MM_D_CCW | MM_PC_YES | MM_NR_YES,
				NULL, NULL, NULL,
				100, 25,
				500, 800,
				100, 50, 100,
				1000
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
    uint8_t ucRepeat;
    uint8_t ucRepeatStop;
} ST_StepMotorTest;

static ST_StepMotorTest s_stStepMotorTest = { 0, 0, 0, 0 };


static int CmdStepMotorTest(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Stage: %d\n", s_stStepMotorTest.ucStage);
		pifLog_Printf(LT_NONE, "  Operation: %d\n", g_motor.parent._operation);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (!strcmp(argv[0], "op")) {
			int value = atoi(argv[1]);
			if (value >= SMO_2P_4W_1S && value <= SMO_2P_4W_1_2S) {
				pifStepMotor_SetOperation(&g_motor.parent, (PifStepMotorOperation)value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "stage")) {
			int value = atoi(argv[1]);
			if (!value) {
				s_stStepMotorTest.ucStage = 0;
				pifStepMotorPos_Stop(&g_motor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= STEP_MOTOR_STAGE_COUNT) {
				if (!s_stStepMotorTest.ucStage) {
					s_stStepMotorTest.ucStage = value;
					pifStepMotorPos_Start(&g_motor, s_stStepMotorTest.ucStage - 1, 2000);
				}
				else {
					pifLog_Printf(LT_NONE, "Error: Stage=%d\n", s_stStepMotorTest.ucStage);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "repeat")) {
			int value = atoi(argv[1]);
			switch (value) {
			case 0:
			    s_stStepMotorTest.ucRepeatStop = 1;
				return PIF_LOG_CMD_NO_ERROR;

			case 1:
			    s_stStepMotorTest.ucRepeat = 1;
			    s_stStepMotorTest.ucRepeatStop = 0;
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "off")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorPos_Stop(&g_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "em")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorPos_Emergency(&g_motor);
		}
		else if (!strcmp(argv[0], "init")) {
			pifLog_Printf(LT_INFO, "Init Pos");
		    s_stStepMotorTest.ucInitPos = 1;
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtStable(PifStepMotor *pstOwner)
{
	PifStepMotorPos* pstChild = (PifStepMotorPos*)pstOwner;

	pifLog_Printf(LT_INFO, "EventStable(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstOwner->_current_pulse);
}

static void _evtStop(PifStepMotor *pstOwner)
{
	PifStepMotorPos* pstChild = (PifStepMotorPos*)pstOwner;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventStop(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstOwner->_current_pulse);
}

static void _evtError(PifStepMotor *pstOwner)
{
	PifStepMotorPos* pstChild = (PifStepMotorPos*)pstOwner;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventError(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstOwner->_current_pulse);
}

static uint32_t _taskInitPos(PifTask *pstTask)
{
	static uint32_t unTime;

	(void)pstTask;

	switch (s_stStepMotorTest.ucInitPos) {
	case 1:
		unTime = 500;
		s_stStepMotorTest.ucStage = 0;
		s_stStepMotorTest.ucInitPos = 2;
		pifLog_Printf(LT_INFO, "InitPos: Start");
		break;

	case 2:
		if (!s_stStepMotorTest.ucStage) {
			if (g_switch[0].parent._curr_state == TRUE) {
				pifLog_Printf(LT_INFO, "InitPos: Find");
				s_stStepMotorTest.ucInitPos = 0;
			}
			else {
				if (pifStepMotorPos_Start(&g_motor, 0, unTime)) {
					s_stStepMotorTest.ucStage = 1;
					s_stStepMotorTest.ucInitPos = 3;
					unTime += 500;
				}
				else {
					s_stStepMotorTest.ucInitPos = 4;
				}
			}
		}
		break;

	case 3:
		if (!s_stStepMotorTest.ucStage) {
			if (pifStepMotorPos_Start(&g_motor, 1, unTime)) {
				s_stStepMotorTest.ucStage = 2;
				s_stStepMotorTest.ucInitPos = 2;
				unTime += 500;
			}
			else {
				s_stStepMotorTest.ucInitPos = 4;
			}
		}
		break;

	case 4:
		pifLog_Printf(LT_ERROR, "InitPos: Error");
		s_stStepMotorTest.ucInitPos = 0;
		break;
	}
	return 0;
}

static uint32_t _taskRepeat(PifTask *pstTask)
{
	(void)pstTask;

	switch (s_stStepMotorTest.ucRepeat) {
	case 1:
		s_stStepMotorTest.ucStage = 0;
		s_stStepMotorTest.ucRepeat = 2;
		pifLog_Printf(LT_INFO, "Repeat: Start");
		break;

	case 2:
		if (!s_stStepMotorTest.ucStage) {
			if (s_stStepMotorTest.ucRepeatStop) {
				s_stStepMotorTest.ucRepeat = 5;
			}
			else if (pifStepMotorPos_Start(&g_motor, 2, 0)) {
				s_stStepMotorTest.ucStage = 3;
				s_stStepMotorTest.ucRepeat = 3;
			}
			else {
				s_stStepMotorTest.ucRepeat = 4;
			}
		}
		break;

	case 3:
		if (!s_stStepMotorTest.ucStage) {
			if (s_stStepMotorTest.ucRepeatStop) {
				s_stStepMotorTest.ucRepeat = 5;
			}
			if (pifStepMotorPos_Start(&g_motor, 3, 0)) {
				s_stStepMotorTest.ucStage = 2;
				s_stStepMotorTest.ucRepeat = 2;
			}
			else {
				s_stStepMotorTest.ucRepeat = 4;
			}
		}
		break;

	case 4:
		pifLog_Printf(LT_ERROR, "Repeat: Error");
		s_stStepMotorTest.ucRepeat = 0;
		s_stStepMotorTest.ucRepeatStop = 0;
		break;

	case 5:
		pifLog_Printf(LT_ERROR, "Repeat: Stop");
		s_stStepMotorTest.ucRepeat = 0;
		s_stStepMotorTest.ucRepeatStop = 0;
		break;
	}
	return 0;
}

// One task polls every switch, instead of a task for each of them.
static uint32_t _taskSwitch(PifTask *pstTask)
{
	(void)pstTask;

	for (int i = 0; i < SWITCH_COUNT; i++) {
		pifSensorSwitch_ProcessAcquire(&g_switch[i]);
	}
	return 0;
}

BOOL appSetup()
{
	PifTask* p_task;

	if (!pifLog_UseCommand(32, c_psCmdTable, "\nDebug> ")) goto fail;										// 32bytes

    g_motor.parent.evt_stable = _evtStable;
    g_motor.parent.evt_stop = _evtStop;
    g_motor.parent.evt_error = _evtError;
    pifStepMotorPos_AddStages(&g_motor, STEP_MOTOR_STAGE_COUNT, s_stStepMotorStages);

    p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskSwitch, NULL, TRUE);							// 1ms
    if (!p_task) goto fail;
    p_task->name = "Switch";

    p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 10000, _taskInitPos, NULL, TRUE);						// 10ms
    if (!p_task) goto fail;
    p_task->name = "InitPos";

    p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 10000, _taskRepeat, NULL, TRUE);						// 10ms
    if (!p_task) goto fail;
    p_task->name = "Repeat";

    if (!pifLed_AttachSBlink(&g_led_l, 500)) goto fail;														// 500ms
    g_led_l._p_task->name = "LedL";
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***          exStepMotorPos          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
    pifLog_Printf(LT_INFO, "Task=%d/%d Timer1ms=%d/%d Timer200us=%d/%d\n", pifTaskManager_Count(), TASK_SIZE,
    		pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE, pifTimerManager_Count(&g_timer_200us), TIMER_200US_SIZE);
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
