#include "appMain.h"


PifLed g_led_l;
PifStepMotor g_motor;
PifTimerManager g_timer_1ms;
PifTimerManager g_timer_200us;

static int CmdStepMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "mt", CmdStepMotorTest, "Motor Test", NULL },

	{ NULL, NULL, NULL, NULL }
};

typedef struct {
	int nMode;
	uint16_t usBreakTime;
	uint32_t unStepCount;
} ST_StepMotorTest;

static ST_StepMotorTest s_stStepMotorTest = { 0, 1000, 200 };


static int CmdStepMotorTest(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Operation: %d\n", g_motor._operation);
		pifLog_Printf(LT_NONE, "  Direction: %d\n", g_motor._direction);
		pifLog_Printf(LT_NONE, "  P/S: %u\n", g_motor._current_pps);
		pifLog_Printf(LT_NONE, "  R/M: %2f\n", pifStepMotor_GetRpm(&g_motor));
		pifLog_Printf(LT_NONE, "  Step Count: %d\n", s_stStepMotorTest.unStepCount);
		pifLog_Printf(LT_NONE, "  Break Time: %d\n", s_stStepMotorTest.usBreakTime);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (!strcmp(argv[0], "op")) {
			int value = atoi(argv[1]);
			if (value >= SMO_2P_4W_1S && value <= SMO_2P_4W_1_2S) {
				pifStepMotor_SetOperation(&g_motor, (PifStepMotorOperation)value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "pps")) {
			float value = atof(argv[1]);
			if (value > 0) {
				if (!pifStepMotor_SetPps(&g_motor, value)) {
					pifLog_Printf(LT_ERROR, "  Invalid Parameter: %d", value);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "rpm")) {
			float value = atof(argv[1]);
			if (value > 0) {
				if (!pifStepMotor_SetRpm(&g_motor, value)) {
					pifLog_Printf(LT_ERROR, "  Invalid Parameter: %d", value);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "dir")) {
			int value = atoi(argv[1]);
			if (value == 0 || value == 1) {
				pifStepMotor_SetDirection(&g_motor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "cnt")) {
			int value = atoi(argv[1]);
			if (value > 0) {
				s_stStepMotorTest.unStepCount = value;
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "cbt")) {
			int value = atoi(argv[1]);
			if (value > 0) {
				s_stStepMotorTest.usBreakTime = value;
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "str")) {
			s_stStepMotorTest.nMode = 1;
			pifStepMotor_Start(&g_motor, 0);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "sts")) {
			s_stStepMotorTest.nMode = 2;
			pifStepMotor_Start(&g_motor, s_stStepMotorTest.unStepCount);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "sp")) {
			s_stStepMotorTest.nMode = 0;
			pifStepMotor_BreakRelease(&g_motor, s_stStepMotorTest.usBreakTime);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "rel")) {
			pifStepMotor_Release(&g_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtStop(PifStepMotor *pstOwner)
{
	s_stStepMotorTest.nMode = 0;
	pifStepMotor_BreakRelease(pstOwner, s_stStepMotorTest.usBreakTime);
}

BOOL appSetup()
{
    if (!pifLog_UseCommand(32, c_psCmdTable, "\nDebug> ")) return FALSE;	// 32bytes

	g_motor.evt_stop = _evtStop;
	pifStepMotor_SetPps(&g_motor, 200);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;					// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
