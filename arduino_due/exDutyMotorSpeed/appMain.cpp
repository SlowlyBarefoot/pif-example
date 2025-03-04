#include "appMain.h"


PifLed g_led_l;
PifDutyMotorSpeed g_motor;
PifSensorSwitch g_switch[SWITCH_COUNT];
PifTimerManager g_timer_1ms;

static int CmdDutyMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "mt", CmdDutyMotorTest, "Motor Test", NULL },

	{ NULL, NULL, NULL, NULL }
};

#define DUTY_MOTOR_STAGE_COUNT	7

const PifDutyMotorSpeedStage s_stDutyMotorStages[DUTY_MOTOR_STAGE_COUNT] = {
		{
				MM_D_CCW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &g_switch[0].parent,
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_CW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &g_switch[2].parent,
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_CW | MM_CIAS_YES | MM_CFPS_YES,
				&g_switch[0].parent, &g_switch[1].parent, &g_switch[2].parent,
				48, 16,
				200, 0,
				32, 16, 1000
		},
		{
				MM_D_CCW | MM_CIAS_YES | MM_CFPS_YES,
				&g_switch[2].parent, &g_switch[1].parent, &g_switch[0].parent,
				64, 16,
				200, 500,
				32, 16, 1000
		},
		{
				MM_D_CW,
				NULL, NULL, NULL,
				0, 0,
				200, 0,
				32, 16, 1000
		},
		{
				MM_D_CCW,
				NULL, NULL, NULL,
				64, 16,
				200, 0,
				0, 0, 0
		},
		{
				MM_D_CW,
				NULL, NULL, NULL,
				64, 16,
				200, 0,
				0, 0, 2000
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 0, 0 };


static int CmdDutyMotorTest(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Stage: %d\n", s_stDutyMotorTest.ucStage);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (!strcmp(argv[0], "stage")) {
			int value = atoi(argv[1]);
			if (!value) {
				s_stDutyMotorTest.ucStage = 0;
				pifDutyMotorSpeed_Stop(&g_motor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= DUTY_MOTOR_STAGE_COUNT) {
				if (!s_stDutyMotorTest.ucStage) {
					if (pifDutyMotorSpeed_Start(&g_motor, value - 1, 2000)) {
						s_stDutyMotorTest.ucStage = value;
					}
				}
				else {
					pifLog_Printf(LT_NONE, "Error: Stage=%d\n", s_stDutyMotorTest.ucStage);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "off")) {
			pifLog_Printf(LT_INFO, "Stop");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorSpeed_Stop(&g_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "em")) {
			pifLog_Printf(LT_INFO, "Emergency");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorSpeed_Emergency(&g_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "init")) {
			pifLog_Printf(LT_INFO, "Init Pos");
		    s_stDutyMotorTest.ucInitPos = 1;
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtStable(PifDutyMotor *pstOwner)
{
	PifDutyMotorSpeed* pstChild = (PifDutyMotorSpeed*)pstOwner;

	pifLog_Printf(LT_INFO, "EventStable(%d) : S=%u", pstOwner->_id, pstChild->_stage_index);
}

static void _evtStop(PifDutyMotor *pstOwner)
{
	PifDutyMotorSpeed* pstChild = (PifDutyMotorSpeed*)pstOwner;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventStop(%d) : S=%u", pstOwner->_id, pstChild->_stage_index);
}

static void _evtError(PifDutyMotor *pstOwner)
{
	PifDutyMotorSpeed* pstChild = (PifDutyMotorSpeed*)pstOwner;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventError(%d) : S=%u", pstOwner->_id, pstChild->_stage_index);
}

static uint32_t _taskInitPos(PifTask *pstTask)
{
	static uint32_t unTime;

	(void)pstTask;

	switch (s_stDutyMotorTest.ucInitPos) {
	case 1:
		unTime = 200;
		s_stDutyMotorTest.ucStage = 0;
		s_stDutyMotorTest.ucInitPos = 2;
		pifLog_Printf(LT_INFO, "InitPos: Start");
		break;

	case 2:
		if (!s_stDutyMotorTest.ucStage) {
			if (g_switch[0].parent._curr_state == ON) {
				pifLog_Printf(LT_INFO, "InitPos: Find");
				s_stDutyMotorTest.ucInitPos = 0;
			}
			else {
				if (pifDutyMotorSpeed_Start(&g_motor, 0, unTime)) {
					s_stDutyMotorTest.ucStage = 1;
					s_stDutyMotorTest.ucInitPos = 3;
					unTime += 200;
				}
				else {
					s_stDutyMotorTest.ucInitPos = 4;
				}
			}
		}
		break;

	case 3:
		if (!s_stDutyMotorTest.ucStage) {
			if (pifDutyMotorSpeed_Start(&g_motor, 1, unTime)) {
				s_stDutyMotorTest.ucStage = 2;
				s_stDutyMotorTest.ucInitPos = 2;
				unTime += 200;
			}
			else {
				s_stDutyMotorTest.ucInitPos = 4;
			}
		}
		break;

	case 4:
		pifLog_Printf(LT_ERROR, "InitPos: Error");
		s_stDutyMotorTest.ucInitPos = 0;
		break;
	}
	return 0;
}

BOOL appSetup()
{
	PifTask* p_task;

    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return FALSE;

    pifDutyMotorSpeed_AddStages(&g_motor, DUTY_MOTOR_STAGE_COUNT, s_stDutyMotorStages);
    g_motor.parent.evt_stable = _evtStable;
    g_motor.parent.evt_stop = _evtStop;
    g_motor.parent.evt_error = _evtError;

    for (int i = 0; i < SWITCH_COUNT; i++) {
    	g_switch[i].parent.p_issuer = &g_motor;
	    if (!pifSensorSwitch_AttachTaskAcquire(&g_switch[i], TM_PERIOD, 1000, TRUE)) return FALSE;	// 1ms
    }

    p_task = pifTaskManager_Add(TM_PERIOD, 10000, _taskInitPos, NULL, TRUE);						// 10ms
    if (!p_task) return FALSE;
    p_task->name = "InitPos";

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;											// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
