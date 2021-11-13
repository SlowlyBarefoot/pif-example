#include "appMain.h"
#include "exStepMotorPos.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_sensor_switch.h"
#include "pif_step_motor_pos.h"


#define SWITCH_COUNT         	3

#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


PifPulse *g_pstTimer1ms = NULL;
PifPulse *g_pstTimer200us = NULL;

static PifStepMotor *s_pstMotor = NULL;
static PifSensor *s_pstSwitch[SWITCH_COUNT] = { NULL, NULL, NULL };

static int CmdStepMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "mt", CmdStepMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define STEP_MOTOR_STAGE_COUNT	5

const PifStepMotorPosStage s_stStepMotorStages[STEP_MOTOR_STAGE_COUNT] = {
		{
				MM_D_CCW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_pstSwitch[0],
				0, 0,
				50, 0,
				0, 0, 100,
				0
		},
		{
				MM_D_CW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_pstSwitch[2],
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
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "\n  Stage: %d", s_stStepMotorTest.ucStage);
		pifLog_Printf(LT_NONE, "\n  Operation: %d", s_pstMotor->_operation);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "op")) {
			int value = atoi(argv[2]);
			if (value >= SMO_2P_4W_1S && value <= SMO_2P_4W_1_2S) {
				pifStepMotor_SetOperation(s_pstMotor, (PifStepMotorOperation)value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "stage")) {
			int value = atoi(argv[2]);
			if (!value) {
				s_stStepMotorTest.ucStage = 0;
				pifStepMotorPos_Stop(s_pstMotor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= STEP_MOTOR_STAGE_COUNT) {
				if (!s_stStepMotorTest.ucStage) {
					s_stStepMotorTest.ucStage = value;
					pifStepMotorPos_Start(s_pstMotor, s_stStepMotorTest.ucStage - 1, 2000);
				}
				else {
					pifLog_Printf(LT_NONE, "\nError: Stage=%d", s_stStepMotorTest.ucStage);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "repeat")) {
			int value = atoi(argv[2]);
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
	else if (argc > 1) {
		if (!strcmp(argv[1], "off")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorPos_Stop(s_pstMotor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorPos_Emergency(s_pstMotor);
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
			if (s_pstSwitch[0]->_curr_state == ON) {
				pifLog_Printf(LT_INFO, "InitPos: Find");
				s_stStepMotorTest.ucInitPos = 0;
			}
			else {
				if (pifStepMotorPos_Start(s_pstMotor, 0, unTime)) {
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
			if (pifStepMotorPos_Start(s_pstMotor, 1, unTime)) {
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

static uint16_t _taskRepeat(PifTask *pstTask)
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
			else if (pifStepMotorPos_Start(s_pstMotor, 2, 0)) {
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
			if (pifStepMotorPos_Start(s_pstMotor, 3, 0)) {
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

void appSetup(PifActTimer1us act_timer1us)
{
	PifComm *pstCommLog;
	PifLed *pstLedL;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(10)) return;

    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000, 1);									// 1000us
    if (!g_pstTimer1ms) return;

    g_pstTimer200us = pifPulse_Create(PIF_ID_AUTO, 200, 3);									// 200us
    if (!g_pstTimer200us) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;						// 1ms
	pstCommLog->act_receive_data = actLogReceiveData;
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;											// 500ms

    for (int i = 0; i < SWITCH_COUNT; i++) {
		s_pstSwitch[i] = pifSensorSwitch_Create(PIF_ID_SWITCH + i, 0);
		if (!s_pstSwitch[i]) return;
	    if (!pifSensorSwitch_AttachTask(s_pstSwitch[i], TM_PERIOD_MS, 1, TRUE)) return;		// 1ms
	    pifSensor_AttachAction(s_pstSwitch[i], actPhotoInterruptAcquire);
    }

    s_pstMotor = pifStepMotorPos_Create(PIF_ID_AUTO, g_pstTimer200us,
    		STEP_MOTOR_RESOLUTION, SMO_2P_4W_1S, 100);										// 100ms
    if (!s_pstMotor) return;
    s_pstMotor->act_set_step = actSetStep;
    s_pstMotor->evt_stable = _evtStable;
    s_pstMotor->evt_stop = _evtStop;
    s_pstMotor->evt_error = _evtError;
    pifStepMotor_SetReductionGearRatio(s_pstMotor, STEP_MOTOR_REDUCTION_GEAR_RATIO);
    pifStepMotorPos_AddStages(s_pstMotor, STEP_MOTOR_STAGE_COUNT, s_stStepMotorStages);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 10, _taskInitPos, NULL, TRUE)) return;			// 10ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 10, _taskRepeat, NULL, TRUE)) return;				// 10ms

    pifLed_BlinkOn(pstLedL, 0);

	pifLog_Printf(LT_INFO, "Task=%d Pulse1ms=%d Pulse200us=%d\n", pifTaskManager_Count(),
			pifPulse_Count(g_pstTimer1ms), pifPulse_Count(g_pstTimer200us));
}
