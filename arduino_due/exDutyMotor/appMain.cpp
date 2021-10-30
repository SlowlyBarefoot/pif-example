#include "appMain.h"
#include "exDutyMotor.h"

#include "pif_duty_motor.h"
#include "pif_led.h"
#include "pif_log.h"


PifPulse *g_pstTimer1ms = NULL;

static PifDutyMotor *s_pstMotor = NULL;

static int CmdDutyMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
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
		pifLog_Printf(LT_NONE, "\n  Duty: %d", s_pstMotor->_current_duty);
		pifLog_Printf(LT_NONE, "\n  Direction: %d", s_pstMotor->_direction);
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
	PifComm *pstCommLog;
	PifLed *pstLedL;

	pif_Init(NULL);
	pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;		// 1ms
	pstCommLog->act_receive_data = actLogReceiveData;
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    s_pstMotor = pifDutyMotor_Create(PIF_ID_AUTO, g_pstTimer1ms, 255);
    if (!s_pstMotor) return;
    s_pstMotor->act_set_duty = actSetDuty;
    s_pstMotor->act_set_direction = actSetDirection;
    s_pstMotor->act_operate_break = actOperateBreak;
}
