#include "appMain.h"
#include "exDutyMotorSpeedEnc.h"

#include "pifDutyMotorSpeedEnc.h"
#include "pifLed.h"
#include "pifLog.h"
#include "pifSensorSwitch.h"


#define SWITCH_COUNT         	3


PifDutyMotor *g_pstMotor = NULL;
PifPulse *g_pstTimer1ms = NULL;

static PifSensor *s_pstSwitch[SWITCH_COUNT] = { NULL, NULL, NULL };

static int CmdDutyMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "mt", CmdDutyMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define DUTY_MOTOR_STAGE_COUNT	4

const PIF_stDutyMotorSpeedEncStage s_stDutyMotorStages[DUTY_MOTOR_STAGE_COUNT] = {
		{
				MM_D_CCW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_pstSwitch[0],
				0, 0, 0, 0,
				500, 50, 0, 0, 90, 110,
				0, 0, 100
		},
		{
				MM_D_CW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_pstSwitch[2],
				0, 0, 0, 0,
				500, 50, 0, 0, 90, 110,
				0, 0, 100
		},
		{
				MM_D_CW | MM_SC_YES | MM_CIAS_YES,
				&s_pstSwitch[0], &s_pstSwitch[1], &s_pstSwitch[2],
				95, 48, 16, 5000,
				2300, 230, 0, 3000, 90, 110,
				50, 16, 1000
		},
		{
				MM_D_CCW | MM_SC_YES | MM_CIAS_YES,
				&s_pstSwitch[2], &s_pstSwitch[1], &s_pstSwitch[0],
				95, 48, 16, 5000,
				2300, 230, 0, 3000, 90, 110,
				50, 16, 0
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
				pifDutyMotorSpeedEnc_Stop(g_pstMotor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= DUTY_MOTOR_STAGE_COUNT) {
				if (!s_stDutyMotorTest.ucStage) {
					if (pifDutyMotorSpeedEnc_Start(g_pstMotor, value - 1, 1000)) {
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
			pifDutyMotorSpeedEnc_Stop(g_pstMotor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			pifLog_Printf(LT_INFO, "Emergency");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorSpeedEnc_Emergency(g_pstMotor);
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
	PIF_stDutyMotorSpeedEnc* pstChild = (PIF_stDutyMotorSpeedEnc*)pstOwner;

	pifLog_Printf(LT_INFO, "EventStable(%d) : S=%u", pstOwner->_id, pstChild->_ucStageIndex);
}

static void _evtStop(PifDutyMotor *pstOwner)
{
	PIF_stDutyMotorSpeedEnc* pstChild = (PIF_stDutyMotorSpeedEnc*)pstOwner;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventStop(%d) : S=%u", pstOwner->_id, pstChild->_ucStageIndex);
}

static void _evtError(PifDutyMotor *pstOwner)
{
	PIF_stDutyMotorSpeedEnc* pstChild = (PIF_stDutyMotorSpeedEnc*)pstOwner;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventError(%d) : S=%u", pstOwner->_id, pstChild->_ucStageIndex);
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
				if (pifDutyMotorSpeedEnc_Start(g_pstMotor, 0, unTime)) {
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
			if (pifDutyMotorSpeedEnc_Start(g_pstMotor, 1, unTime)) {
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
	PifComm *pstCommLog;
	PifLed *pstLedL;

	pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);										// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;					// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;					// 1ms
	pifComm_AttachActReceiveData(pstCommLog, actLogReceiveData);
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;											// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    for (int i = 0; i < SWITCH_COUNT; i++) {
		s_pstSwitch[i] = pifSensorSwitch_Create(PIF_ID_SWITCH + i, 0);
		if (!s_pstSwitch[i]) return;
	    if (!pifSensorSwitch_AttachTask(s_pstSwitch[i], TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	    pifSensor_AttachAction(s_pstSwitch[i], actPhotoInterruptAcquire);
    }

    g_pstMotor = pifDutyMotorSpeedEnc_Create(PIF_ID_AUTO, g_pstTimer1ms, 255, 100);
    pifDutyMotorSpeedEnc_AddStages(g_pstMotor, DUTY_MOTOR_STAGE_COUNT, s_stDutyMotorStages);
    pifDutyMotor_AttachAction(g_pstMotor, actSetDuty, actSetDirection, actOperateBreak);
    g_pstMotor->evt_stable = _evtStable;
    g_pstMotor->evt_stop = _evtStop;
    g_pstMotor->evt_error = _evtError;
    pifPidControl_Init(pifDutyMotorSpeedEnc_GetPidControl(g_pstMotor), 0.1, 0, 0, 100);

    if (!pifLog_AttachTask(TM_PERIOD_MS, 20, TRUE)) return;								// 20ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 10, _taskInitPos, NULL, TRUE)) return;			// 10ms
}
