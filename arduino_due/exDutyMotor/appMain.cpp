#include "appMain.h"
#include "exDutyMotor.h"

#include "pifDutyMotor.h"
#include "pifLed.h"
#include "pifLog.h"
#include "pifTerminal.h"


#define COMM_COUNT				1
#define LED_COUNT         		1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              3


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stDutyMotor *s_pstMotor = NULL;

static int CmdDutyMotorTest(int argc, char *argv[]);

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },
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
		return PIF_TERM_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "duty")) {
			int value = atoi(argv[2]);
			if (value > 0 && value < 256) {
				pifDutyMotor_SetDuty(s_pstMotor, value);
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "dir")) {
			int value = atoi(argv[2]);
			if (value == 0 || value == 1) {
				pifDutyMotor_SetDirection(s_pstMotor, value);
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "stop")) {
			pifDutyMotor_BreakRelease(s_pstMotor, 0);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "break")) {
			pifDutyMotor_BreakRelease(s_pstMotor, 1000);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "start")) {
			pifDutyMotor_Start(s_pstMotor, s_stDutyMotorTest.usDuty);
			return PIF_TERM_CMD_NO_ERROR;
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	return PIF_TERM_CMD_TOO_FEW_ARGS;
}

void appSetup()
{
	PIF_stLed *pstLedL;
	PIF_stComm *pstSerial;

	pif_Init(NULL);

	pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;
    pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!pstSerial) return;
	pifComm_AttachActReceiveData(pstSerial, actSerialReceiveData);
	pifComm_AttachActSendData(pstSerial, actSerialSendData);

    if (!pifTerminal_Init(c_psCmdTable, "\nDebug")) return;
	pifTerminal_AttachComm(pstSerial);

	pifLog_DetachActPrint();
    pifLog_UseTerminal(TRUE);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);	// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;						// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    if (!pifDutyMotor_Init(g_pstTimer1ms, MOTOR_COUNT)) return;
    s_pstMotor = pifDutyMotor_Add(PIF_ID_AUTO, 255);
    if (!s_pstMotor) return;
    pifDutyMotor_AttachAction(s_pstMotor, actSetDuty, actSetDirection, actOperateBreak);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;			// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;			// 1ms
}
