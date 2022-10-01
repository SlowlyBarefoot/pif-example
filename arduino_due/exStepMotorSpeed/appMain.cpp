#include "appMain.h"
#include "exStepMotorSpeed.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_sensor_switch.h"
#include "pif_step_motor_speed.h"


#define SWITCH_COUNT         	3

#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


PifTimerManager g_timer_1ms;
PifTimerManager g_timer_200us;

static PifStepMotorSpeed s_motor;
static PifSensorSwitch s_switch[SWITCH_COUNT];

static int CmdStepMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "mt", CmdStepMotorTest, "Motor Test" },

	{ NULL, NULL, NULL }
};

#define STEP_MOTOR_STAGE_COUNT	4

const PifStepMotorSpeedStage s_stStepMotorStages[STEP_MOTOR_STAGE_COUNT] = {
		{
				MM_D_CCW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_switch[0].parent,
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_CW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_switch[2].parent,
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_CW,
				NULL, &s_switch[1].parent, NULL,
				100, 25,
				500, 0,
				100, 50, 100
		},
		{
				MM_D_CCW,
				NULL, &s_switch[1].parent, NULL,
				100, 50,
				500, 0,
				100, 25, 100
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
} ST_StepMotorTest;

static ST_StepMotorTest s_stStepMotorTest = { 0, 0 };


static int CmdStepMotorTest(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "  Stage: %d\n", s_stStepMotorTest.ucStage);
		pifLog_Printf(LT_NONE, "  Operation: %d\n", s_motor.parent._operation);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "op")) {
			int value = atoi(argv[2]);
			if (value >= SMO_2P_4W_1S && value <= SMO_2P_4W_1_2S) {
				pifStepMotor_SetOperation(&s_motor.parent, (PifStepMotorOperation)value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "stage")) {
			int value = atoi(argv[2]);
			if (!value) {
				s_stStepMotorTest.ucStage = 0;
				pifStepMotorSpeed_Stop(&s_motor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= STEP_MOTOR_STAGE_COUNT) {
				if (!s_stStepMotorTest.ucStage) {
					s_stStepMotorTest.ucStage = value;
					pifStepMotorSpeed_Start(&s_motor, s_stStepMotorTest.ucStage - 1, 2000);
				}
				else {
					pifLog_Printf(LT_NONE, "Error: Stage=%d\n", s_stStepMotorTest.ucStage);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "off")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorSpeed_Stop(&s_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorSpeed_Emergency(&s_motor);
		}
		else if (!strcmp(argv[1], "init")) {
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
	PifStepMotorSpeed* pstChild = (PifStepMotorSpeed*)pstOwner;

	pifLog_Printf(LT_INFO, "EventStable(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstOwner->_current_pulse);
}

static void _evtStop(PifStepMotor *pstOwner)
{
	PifStepMotorSpeed* pstChild = (PifStepMotorSpeed *)pstOwner;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventStop(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstOwner->_current_pulse);
}

static void _evtError(PifStepMotor *pstOwner)
{
	PifStepMotorSpeed* pstChild = (PifStepMotorSpeed*)pstOwner;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventError(%d) : S=%u P=%u", pstOwner->_id, pstChild->_stage_index, pstOwner->_current_pulse);
}

static uint16_t _taskInitPos(PifTask *pstTask)
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
			if (s_switch[0].parent._curr_state == ON) {
				pifLog_Printf(LT_INFO, "InitPos: Find");
				s_stStepMotorTest.ucInitPos = 0;
			}
			else {
				if (pifStepMotorSpeed_Start(&s_motor, 0, unTime)) {
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
			if (pifStepMotorSpeed_Start(&s_motor, 1, unTime)) {
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

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	static PifLed s_led_l;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(9)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;						// 1000us

    if (!pifTimerManager_Init(&g_timer_200us, PIF_ID_AUTO, 200, 3)) return;						// 200us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;						// 1ms
	s_comm_log.act_receive_data = actLogReceiveData;
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;											// 500ms

    if (!pifStepMotorSpeed_Init(&s_motor, PIF_ID_AUTO, &g_timer_200us, STEP_MOTOR_RESOLUTION,
			SMO_2P_4W_1S, 100)) return;															// 100ms
    s_motor.parent.act_set_step = actSetStep;
    s_motor.parent.evt_stable = _evtStable;
    s_motor.parent.evt_stop = _evtStop;
    s_motor.parent.evt_error = _evtError;
    pifStepMotor_SetReductionGearRatio(&s_motor.parent, STEP_MOTOR_REDUCTION_GEAR_RATIO);
    pifStepMotorSpeed_AddStages(&s_motor, STEP_MOTOR_STAGE_COUNT, s_stStepMotorStages);

    for (int i = 0; i < SWITCH_COUNT; i++) {
		if (!pifSensorSwitch_Init(&s_switch[i], PIF_ID_SWITCH + i, 0, actPhotoInterruptAcquire, &s_motor)) return;
	    if (!pifSensorSwitch_AttachTaskAcquire(&s_switch[i], TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
    }

    if (!pifTaskManager_Add(TM_PERIOD_MS, 10, _taskInitPos, NULL, TRUE)) return;				// 10ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer 1ms=%d Timer 200us=%d\n", pifTaskManager_Count(),
			pifTimerManager_Count(&g_timer_1ms), pifTimerManager_Count(&g_timer_200us));
}
