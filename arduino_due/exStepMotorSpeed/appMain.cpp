#include "appMain.h"
#include "exStepMotorSpeed.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifSensorSwitch.h"
#include "pifStepMotorSpeed.h"
#include "pifTerminal.h"


#define COMM_COUNT				1
#define LED_COUNT         		1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	2
#define PULSE_ITEM_COUNT    	20
#define SWITCH_COUNT         	3
#define TASK_COUNT              7

#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


PIF_stComm *g_pstSerial = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stPulse *g_pstTimer200us = NULL;

static PIF_stStepMotor *s_pstMotor = NULL;
static PIF_stSensor *s_pstSwitch[SWITCH_COUNT] = { NULL, NULL, NULL };

static int CmdStepMotorTest(int argc, char *argv[]);

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },
	{ "mt", CmdStepMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define STEP_MOTOR_STAGE_COUNT	4

const PIF_stStepMotorSpeedStage s_stStepMotorStages[STEP_MOTOR_STAGE_COUNT] = {
		{
				MM_D_enCCW | MM_RT_enTime | MM_CFPS_enYes,
				NULL, NULL, &s_pstSwitch[0],
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_enCW | MM_RT_enTime | MM_CFPS_enYes,
				NULL, NULL, &s_pstSwitch[2],
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_enCW,
				NULL, &s_pstSwitch[1], NULL,
				100, 25,
				500, 0,
				100, 50, 100
		},
		{
				MM_D_enCCW,
				NULL, &s_pstSwitch[1], NULL,
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
		pifLog_Printf(LT_enNone, "\n  Stage: %d", s_stStepMotorTest.ucStage);
		pifLog_Printf(LT_enNone, "\n  Method: %d", s_pstMotor->_enMethod);
		pifLog_Printf(LT_enNone, "\n  Operation: %d", s_pstMotor->_enOperation);
		return PIF_TERM_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "mt")) {
			if (s_pstMotor->_enState == MS_enIdle) {
				int value = atoi(argv[2]);
				if (value >= 0 && value <= 1) {
					pifStepMotor_SetMethod(s_pstMotor, (PIF_enStepMotorMethod)value);
					return PIF_TERM_CMD_NO_ERROR;
				}
			}
		}
		else if (!strcmp(argv[1], "op")) {
			int value = atoi(argv[2]);
			if (value >= SMO_en2P_4W_1S && value <= SMO_en2P_4W_1_2S) {
				pifStepMotor_SetOperation(s_pstMotor, (PIF_enStepMotorOperation)value);
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "stage")) {
			int value = atoi(argv[2]);
			if (!value) {
				s_stStepMotorTest.ucStage = 0;
				pifStepMotorSpeed_Stop(s_pstMotor);
				return PIF_TERM_CMD_NO_ERROR;
			}
			else if (value <= STEP_MOTOR_STAGE_COUNT) {
				if (!s_stStepMotorTest.ucStage) {
					s_stStepMotorTest.ucStage = value;
					pifStepMotorSpeed_Start(s_pstMotor, s_stStepMotorTest.ucStage - 1, 2000);
				}
				else {
					pifLog_Printf(LT_enNone, "\nError: Stage=%d", s_stStepMotorTest.ucStage);
				}
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "off")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorSpeed_Stop(s_pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorSpeed_Emergency(s_pstMotor);
		}
		else if (!strcmp(argv[1], "init")) {
			pifLog_Printf(LT_enInfo, "Init Pos");
		    s_stStepMotorTest.ucInitPos = 1;
			return PIF_TERM_CMD_NO_ERROR;
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	return PIF_TERM_CMD_TOO_FEW_ARGS;
}

static void _evtStable(PIF_stStepMotor *pstOwner, void *pvInfo)
{
	PIF_stStepMotorSpeed *pstInfo = (PIF_stStepMotorSpeed *)pvInfo;

	pifLog_Printf(LT_enInfo, "EventStable(%d) : S=%u P=%u", pstOwner->_usPifId, pstInfo->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static void _evtStop(PIF_stStepMotor *pstOwner, void *pvInfo)
{
	PIF_stStepMotorSpeed *pstInfo = (PIF_stStepMotorSpeed *)pvInfo;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventStop(%d) : S=%u P=%u", pstOwner->_usPifId, pstInfo->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static void _evtError(PIF_stStepMotor *pstOwner, void *pvInfo)
{
	PIF_stStepMotorSpeed *pstInfo = (PIF_stStepMotorSpeed *)pvInfo;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventError(%d) : S=%u P=%u", pstOwner->_usPifId, pstInfo->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static void _taskInitPos(PIF_stTask *pstTask)
{
	static uint32_t unTime;

	(void)pstTask;

	switch (s_stStepMotorTest.ucInitPos) {
	case 1:
		unTime = 500;
		s_stStepMotorTest.ucStage = 0;
		s_stStepMotorTest.ucInitPos = 2;
		pifLog_Printf(LT_enInfo, "InitPos: Start");
		break;

	case 2:
		if (!s_stStepMotorTest.ucStage) {
			if (s_pstSwitch[0]->_swCurrState == ON) {
				pifLog_Printf(LT_enInfo, "InitPos: Find");
				s_stStepMotorTest.ucInitPos = 0;
			}
			else {
				if (pifStepMotorSpeed_Start(s_pstMotor, 0, unTime)) {
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
			if (pifStepMotorSpeed_Start(s_pstMotor, 1, unTime)) {
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
		pifLog_Printf(LT_enError, "InitPos: Error");
		s_stStepMotorTest.ucInitPos = 0;
		break;
	}
}

void appSetup()
{
	PIF_stLed *pstLedL = NULL;
	PIF_stTask *pstTask;

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;
    g_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!g_pstSerial) return;

    if (!pifTerminal_Init(c_psCmdTable, "\nDebug")) return;
	pifTerminal_AttachComm(g_pstSerial);

	pifLog_DetachActPrint();
    pifLog_UseTerminal(TRUE);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);								// 1000us
    if (!g_pstTimer1ms) return;
    g_pstTimer200us = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 200);								// 200us
    if (!g_pstTimer200us) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;													// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;
    for (int i = 0; i < SWITCH_COUNT; i++) {
		s_pstSwitch[i] = pifSensorSwitch_Add(PIF_ID_SWITCH + i, 0);
		if (!s_pstSwitch[i]) return;
	    pifSensor_AttachAction(s_pstSwitch[i], actPhotoInterruptAcquire);
    }

    if (!pifStepMotor_Init(g_pstTimer200us, MOTOR_COUNT)) return;
    s_pstMotor = pifStepMotorSpeed_Add(PIF_ID_AUTO, STEP_MOTOR_RESOLUTION, SMO_en2P_4W_1S, 100);	// 100ms
    if (!s_pstMotor) return;
    pifStepMotor_AttachAction(s_pstMotor, actSetStep);
    s_pstMotor->evtStable = _evtStable;
    s_pstMotor->evtStop = _evtStop;
    s_pstMotor->evtError = _evtError;
    pifStepMotor_SetReductionGearRatio(s_pstMotor, STEP_MOTOR_REDUCTION_GEAR_RATIO);
    pifStepMotorSpeed_AddStages(s_pstMotor, STEP_MOTOR_STAGE_COUNT, s_stStepMotorStages);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;										// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;										// 1ms
    if (!pifTask_AddPeriodMs(1, pifSensorSwitch_taskAll, NULL)) return;								// 1ms
    pstTask = pifTask_AddPeriodUs(200, pifStepMotor_taskAll, NULL);									// 200us
    if (!pstTask) return;
    pifStepMotor_AttachTask(s_pstMotor, pstTask);

    if (!pifTask_AddPeriodMs(1, taskTerminal, NULL)) return;										// 1ms
    if (!pifTask_AddPeriodMs(10, _taskInitPos, NULL)) return;										// 10ms
}
