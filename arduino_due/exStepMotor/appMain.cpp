#include "appMain.h"
#include "exStepMotor.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifStepMotor.h"
#include "pifTerminal.h"


#define COMM_COUNT				1
#define LED_COUNT         		1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	2
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              4

#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


PIF_stComm *g_pstSerial = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stPulse *g_pstTimer200us = NULL;

static PIF_stStepMotor *s_pstMotor = NULL;

static int CmdStepMotorTest(int argc, char *argv[]);

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },
	{ "mt", CmdStepMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

typedef struct {
	int nMode;
	uint16_t usBreakTime;
	uint32_t unStepCount;
} ST_StepMotorTest;

static ST_StepMotorTest s_stStepMotorTest = { 0, 1000, 200 };


static int CmdStepMotorTest(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_enNone, "\n  Method: %d", s_pstMotor->_enMethod);
		pifLog_Printf(LT_enNone, "\n  Operation: %d", s_pstMotor->_enOperation);
		pifLog_Printf(LT_enNone, "\n  Direction: %d", s_pstMotor->_ucDirection);
		pifLog_Printf(LT_enNone, "\n  P/S: %u", s_pstMotor->_usCurrentPps);
		pifLog_Printf(LT_enNone, "\n  R/M: %2f", pifStepMotor_GetRpm(s_pstMotor));
		pifLog_Printf(LT_enNone, "\n  Step Count: %d", s_stStepMotorTest.unStepCount);
		pifLog_Printf(LT_enNone, "\n  Break Time: %d", s_stStepMotorTest.usBreakTime);
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
		else if (!strcmp(argv[1], "pps")) {
			float value = atof(argv[2]);
			if (value > 0) {
				if (!pifStepMotor_SetPps(s_pstMotor, value)) {
					pifLog_Printf(LT_enError, "\n  Invalid Parameter: %d", value);
				}
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "rpm")) {
			float value = atof(argv[2]);
			if (value > 0) {
				if (!pifStepMotor_SetRpm(s_pstMotor, value)) {
					pifLog_Printf(LT_enError, "\n  Invalid Parameter: %d", value);
				}
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "dir")) {
			int value = atoi(argv[2]);
			if (value == 0 || value == 1) {
				pifStepMotor_SetDirection(s_pstMotor, value);
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "cnt")) {
			int value = atoi(argv[2]);
			if (value > 0) {
				s_stStepMotorTest.unStepCount = value;
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "cbt")) {
			int value = atoi(argv[2]);
			if (value > 0) {
				s_stStepMotorTest.usBreakTime = value;
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "str")) {
			s_stStepMotorTest.nMode = 1;
			pifStepMotor_Start(s_pstMotor, 0);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "sts")) {
			s_stStepMotorTest.nMode = 2;
			pifStepMotor_Start(s_pstMotor, s_stStepMotorTest.unStepCount);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "sp")) {
			s_stStepMotorTest.nMode = 0;
			pifStepMotor_BreakRelease(s_pstMotor, s_stStepMotorTest.usBreakTime);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "rel")) {
			pifStepMotor_Release(s_pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	return PIF_TERM_CMD_TOO_FEW_ARGS;
}

static void _evtStop(PIF_stStepMotor *pstOwner)
{
	s_stStepMotorTest.nMode = 0;
	pifStepMotor_BreakRelease(pstOwner, s_stStepMotorTest.usBreakTime);
}

void appSetup()
{
	PIF_stLed *pstLedL = NULL;
	PIF_stTask *pstTask;

	pif_Init(NULL);

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
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    g_pstTimer200us = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 200);		// 200us
    if (!g_pstTimer200us) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    if (!pifStepMotor_Init(g_pstTimer200us, MOTOR_COUNT)) return;
    s_pstMotor = pifStepMotor_Add(PIF_ID_AUTO, STEP_MOTOR_RESOLUTION, SMO_en2P_4W_1S);
    if (!s_pstMotor) return;
    pifStepMotor_AttachAction(s_pstMotor, actSetStep);
    s_pstMotor->evtStop = _evtStop;
    pifStepMotor_SetReductionGearRatio(s_pstMotor, STEP_MOTOR_REDUCTION_GEAR_RATIO);
	pifStepMotor_SetPps(s_pstMotor, 200);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;				// 1ms
    pstTask = pifTask_AddPeriodUs(200, pifStepMotor_taskAll, NULL);			// 200us
    if (!pstTask) return;
    pifStepMotor_AttachTask(s_pstMotor, pstTask);

    if (!pifTask_AddPeriodMs(1, taskTerminal, NULL)) return;				// 1ms
}
