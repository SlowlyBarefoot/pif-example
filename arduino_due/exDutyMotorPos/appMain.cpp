#include "appMain.h"
#include "exDutyMotorPos.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_sensor_switch.h"


#define SWITCH_COUNT         	3


PifDutyMotorPos g_motor;
PifTimerManager g_timer_1ms;

static PifSensorSwitch s_switch[SWITCH_COUNT];

static int CmdDutyMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "mt", CmdDutyMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define DUTY_MOTOR_STAGE_COUNT	4

const PifDutyMotorPosStage s_stDutyMotorStages[DUTY_MOTOR_STAGE_COUNT] = {
		{
				MM_D_CCW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_switch[0].parent,
				0, 0,
				50, 0,
				0, 0, 100,
				0
		},
		{
				MM_D_CW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_switch[2].parent,
				0, 0,
				50, 0,
				0, 0, 100,
				0
		},
		{
				MM_D_CW | MM_PC_YES | MM_CIAS_YES,
				&s_switch[0].parent, &s_switch[1].parent, &s_switch[2].parent,
				50, 50,
				200, 50000,
				40, 50, 500,
				70000
		},
		{
				MM_D_CCW | MM_PC_YES | MM_CIAS_YES,
				&s_switch[2].parent, &s_switch[1].parent, &s_switch[0].parent,
				50, 50,
				200, 50000,
				40, 50, 500,
				70000
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 0, 0 };


static int CmdDutyMotorTest(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "\n  Stage: %d", s_stDutyMotorTest.ucStage);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "stage")) {
			int value = atoi(argv[2]);
			if (!value) {
				s_stDutyMotorTest.ucStage = 0;
				pifDutyMotorPos_Stop(&g_motor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= DUTY_MOTOR_STAGE_COUNT) {
				if (!s_stDutyMotorTest.ucStage) {
					if (pifDutyMotorPos_Start(&g_motor, value - 1, 2000)) {
						s_stDutyMotorTest.ucStage = value;
					}
				}
				else {
					pifLog_Printf(LT_NONE, "\nError: Stage=%d", s_stDutyMotorTest.ucStage);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "off")) {
			pifLog_Printf(LT_INFO, "Stop");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorPos_Stop(&g_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			pifLog_Printf(LT_INFO, "Emergency");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorPos_Emergency(&g_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "init")) {
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
	PifDutyMotorPos* pstChild = (PifDutyMotorPos*)pstOwner;

	pifLog_Printf(LT_INFO, "EventStable(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstChild->_current_pulse);
}

static void _evtStop(PifDutyMotor *pstOwner)
{
	PifDutyMotorPos* pstChild = (PifDutyMotorPos*)pstOwner;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventStop(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstChild->_current_pulse);
}

static void _evtError(PifDutyMotor *pstOwner)
{
	PifDutyMotorPos* pstChild = (PifDutyMotorPos*)pstOwner;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventError(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstChild->_current_pulse);
}

static uint16_t _taskInitPos(PifTask *pstTask)
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
			if (s_switch[0].parent._curr_state == ON) {
				pifLog_Printf(LT_INFO, "InitPos: Find");
				s_stDutyMotorTest.ucInitPos = 0;
			}
			else {
				if (pifDutyMotorPos_Start(&g_motor, 0, unTime)) {
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
			if (pifDutyMotorPos_Start(&g_motor, 1, unTime)) {
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

void appSetup()
{
	static PifComm s_comm_log;
	static PifLed s_led_l;

	pif_Init(NULL);

    if (!pifTaskManager_Init(8)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;				// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
    s_comm_log.act_receive_data = actLogReceiveData;
    s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;										// 500ms

    for (int i = 0; i < SWITCH_COUNT; i++) {
		if (!pifSensorSwitch_Init(&s_switch[i], PIF_ID_SWITCH + i, 0)) return;
	    if (!pifSensorSwitch_AttachTask(&s_switch[i], TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	    pifSensor_AttachAction(&s_switch[i].parent, actPhotoInterruptAcquire);
    }

    if (!pifDutyMotorPos_Init(&g_motor, PIF_ID_AUTO, &g_timer_1ms, 255, 100)) return;	// 100ms
    pifDutyMotorPos_AddStages(&g_motor, DUTY_MOTOR_STAGE_COUNT, s_stDutyMotorStages);
    g_motor.parent.act_set_duty = actSetDuty;
    g_motor.parent.act_set_direction = actSetDirection;
    g_motor.parent.act_operate_break = actOperateBreak;
    g_motor.parent.evt_stable = _evtStable;
    g_motor.parent.evt_stop = _evtStop;
    g_motor.parent.evt_error = _evtError;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 10, _taskInitPos, NULL, TRUE)) return;		// 10ms

    pifLed_BlinkOn(&s_led_l, 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
