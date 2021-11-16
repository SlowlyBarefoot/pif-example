#include "appMain.h"
#include "exDutyMotorSpeed.h"

#include "pif_duty_motor_speed.h"
#include "pif_led.h"
#include "pif_log.h"
#include "pif_sensor_switch.h"


#define SWITCH_COUNT         	3


PifDutyMotor *g_pstMotor = NULL;
PifTimerManager g_timer_1ms;

static PifSensor *s_pstSwitch[SWITCH_COUNT] = { NULL, NULL, NULL };

static int CmdDutyMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "mt", CmdDutyMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define DUTY_MOTOR_STAGE_COUNT	7

const PifDutyMotorSpeedStage s_stDutyMotorStages[DUTY_MOTOR_STAGE_COUNT] = {
		{
				MM_D_CCW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_pstSwitch[0],
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_CW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_pstSwitch[2],
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_CW | MM_CIAS_YES | MM_CFPS_YES,
				&s_pstSwitch[0], &s_pstSwitch[1], &s_pstSwitch[2],
				48, 16,
				200, 0,
				32, 16, 1000
		},
		{
				MM_D_CCW | MM_CIAS_YES | MM_CFPS_YES,
				&s_pstSwitch[2], &s_pstSwitch[1], &s_pstSwitch[0],
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
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "\n  Stage: %d", s_stDutyMotorTest.ucStage);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "stage")) {
			int value = atoi(argv[2]);
			if (!value) {
				s_stDutyMotorTest.ucStage = 0;
				pifDutyMotorSpeed_Stop(g_pstMotor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= DUTY_MOTOR_STAGE_COUNT) {
				if (!s_stDutyMotorTest.ucStage) {
					if (pifDutyMotorSpeed_Start(g_pstMotor, value - 1, 2000)) {
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
			pifDutyMotorSpeed_Stop(g_pstMotor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			pifLog_Printf(LT_INFO, "Emergency");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorSpeed_Emergency(g_pstMotor);
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
			if (s_pstSwitch[0]->_curr_state == ON) {
				pifLog_Printf(LT_INFO, "InitPos: Find");
				s_stDutyMotorTest.ucInitPos = 0;
			}
			else {
				if (pifDutyMotorSpeed_Start(g_pstMotor, 0, unTime)) {
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
			if (pifDutyMotorSpeed_Start(g_pstMotor, 1, unTime)) {
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
		s_pstSwitch[i] = pifSensorSwitch_Create(PIF_ID_SWITCH + i, 0);
		if (!s_pstSwitch[i]) return;
	    if (!pifSensorSwitch_AttachTask(s_pstSwitch[i], TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	    pifSensor_AttachAction(s_pstSwitch[i], actPhotoInterruptAcquire);
    }

    g_pstMotor = pifDutyMotorSpeed_Create(PIF_ID_AUTO, &g_timer_1ms, 255, 100);			// 100ms
    pifDutyMotorSpeed_AddStages(g_pstMotor, DUTY_MOTOR_STAGE_COUNT, s_stDutyMotorStages);
    g_pstMotor->act_set_duty = actSetDuty;
    g_pstMotor->act_set_direction = actSetDirection;
    g_pstMotor->act_operate_break = actOperateBreak;
    g_pstMotor->evt_stable = _evtStable;
    g_pstMotor->evt_stop = _evtStop;
    g_pstMotor->evt_error = _evtError;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 10, _taskInitPos, NULL, TRUE)) return;		// 10ms

    pifLed_BlinkOn(&s_led_l, 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
