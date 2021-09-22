#include "appMain.h"
#include "exDutyMotor.h"

#include "pifDutyMotor.h"
#include "pifLed.h"
#include "pifLog.h"


#define COMM_COUNT				1
#define LED_COUNT         		1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              3


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stDutyMotor *s_pstMotor = NULL;

static int CmdDutyMotorTest(int argc, char *argv[]);

const PIF_stLogCmdEntry c_psCmdTable[] = {
	{ "mt", CmdDutyMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

typedef struct {
	uint16_t usDuty;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 128 };


static int CmdDutyMotorTest(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_enNone, "\n  Duty: %d", s_pstMotor->_usCurrentDuty);
		pifLog_Printf(LT_enNone, "\n  Direction: %d", s_pstMotor->_ucDirection);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "duty")) {
			int value = atoi(argv[2]);
			if (value > 0 && value < 256) {
				pifDutyMotor_SetDuty(s_pstMotor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "dir")) {
			int value = atoi(argv[2]);
			if (value == 0 || value == 1) {
				pifDutyMotor_SetDirection(s_pstMotor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "stop")) {
			pifDutyMotor_BreakRelease(s_pstMotor, 0);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "break")) {
			pifDutyMotor_BreakRelease(s_pstMotor, 1000);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "start")) {
			pifDutyMotor_Start(s_pstMotor, s_stDutyMotorTest.usDuty);
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

void appSetup()
{
	PIF_stComm *pstCommLog;
	PIF_stLed *pstLedL;

	pif_Init(NULL);
	pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);				// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_Add(TM_enRatio, 100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;	// 100%

    if (!pifDutyMotor_Init(MOTOR_COUNT, g_pstTimer1ms)) return;
    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_Add(TM_enPeriodMs, 1, pifComm_Task, pstCommLog, TRUE)) return;		// 1ms
	pifComm_AttachActReceiveData(pstCommLog, actLogReceiveData);
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;									// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    s_pstMotor = pifDutyMotor_Add(PIF_ID_AUTO, 255);
    if (!s_pstMotor) return;
    pifDutyMotor_AttachAction(s_pstMotor, actSetDuty, actSetDirection, actOperateBreak);

    if (!pifTask_Add(TM_enPeriodMs, 20, pifLog_Task, NULL, TRUE)) return;			// 20ms
}
