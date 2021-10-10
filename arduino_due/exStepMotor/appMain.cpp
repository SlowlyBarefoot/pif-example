#include "appMain.h"
#include "exStepMotor.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifStepMotor.h"


#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


PifPulse *g_pstTimer1ms = NULL;
PifPulse *g_pstTimer200us = NULL;

static PIF_stStepMotor *s_pstMotor = NULL;

static int CmdStepMotorTest(int argc, char *argv[]);

const PIF_stLogCmdEntry c_psCmdTable[] = {
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
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "mt")) {
			if (s_pstMotor->_enState == MS_enIdle) {
				int value = atoi(argv[2]);
				if (value >= 0 && value <= 1) {
					pifStepMotor_SetMethod(s_pstMotor, (PIF_enStepMotorMethod)value);
					return PIF_LOG_CMD_NO_ERROR;
				}
			}
		}
		else if (!strcmp(argv[1], "op")) {
			int value = atoi(argv[2]);
			if (value >= SMO_en2P_4W_1S && value <= SMO_en2P_4W_1_2S) {
				pifStepMotor_SetOperation(s_pstMotor, (PIF_enStepMotorOperation)value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "pps")) {
			float value = atof(argv[2]);
			if (value > 0) {
				if (!pifStepMotor_SetPps(s_pstMotor, value)) {
					pifLog_Printf(LT_enError, "\n  Invalid Parameter: %d", value);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "rpm")) {
			float value = atof(argv[2]);
			if (value > 0) {
				if (!pifStepMotor_SetRpm(s_pstMotor, value)) {
					pifLog_Printf(LT_enError, "\n  Invalid Parameter: %d", value);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "dir")) {
			int value = atoi(argv[2]);
			if (value == 0 || value == 1) {
				pifStepMotor_SetDirection(s_pstMotor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "cnt")) {
			int value = atoi(argv[2]);
			if (value > 0) {
				s_stStepMotorTest.unStepCount = value;
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "cbt")) {
			int value = atoi(argv[2]);
			if (value > 0) {
				s_stStepMotorTest.usBreakTime = value;
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "str")) {
			s_stStepMotorTest.nMode = 1;
			pifStepMotor_Start(s_pstMotor, 0);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "sts")) {
			s_stStepMotorTest.nMode = 2;
			pifStepMotor_Start(s_pstMotor, s_stStepMotorTest.unStepCount);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "sp")) {
			s_stStepMotorTest.nMode = 0;
			pifStepMotor_BreakRelease(s_pstMotor, s_stStepMotorTest.usBreakTime);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "rel")) {
			pifStepMotor_Release(s_pstMotor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtStop(PIF_stStepMotor *pstOwner)
{
	s_stStepMotorTest.nMode = 0;
	pifStepMotor_BreakRelease(pstOwner, s_stStepMotorTest.usBreakTime);
}

void appSetup(PifActTimer1us act_timer1us)
{
	PIF_stComm *pstCommLog;
	PIF_stLed *pstLedL;

	pif_Init(act_timer1us);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;			// 100%

    g_pstTimer200us = pifPulse_Create(PIF_ID_AUTO, 200);							// 200us
    if (!g_pstTimer200us) return;
    if (!pifPulse_AttachTask(g_pstTimer200us, TM_RATIO, 100, TRUE)) return;		// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	pifComm_AttachActReceiveData(pstCommLog, actLogReceiveData);
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;									// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    s_pstMotor = pifStepMotor_Create(PIF_ID_AUTO, g_pstTimer200us, STEP_MOTOR_RESOLUTION, SMO_en2P_4W_1S);
    if (!s_pstMotor) return;
    if (!pifStepMotor_AttachTask(s_pstMotor, TM_PERIOD_US, 200, FALSE)) return;	// 200us
    pifStepMotor_AttachAction(s_pstMotor, actSetStep);
    s_pstMotor->evtStop = _evtStop;
    pifStepMotor_SetReductionGearRatio(s_pstMotor, STEP_MOTOR_REDUCTION_GEAR_RATIO);
	pifStepMotor_SetPps(s_pstMotor, 200);

    if (!pifLog_AttachTask(TM_PERIOD_MS, 20, TRUE)) return;						// 20ms
}
