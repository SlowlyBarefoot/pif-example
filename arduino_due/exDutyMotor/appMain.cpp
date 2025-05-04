#include "appMain.h"
#include "exDutyMotor.h"


PifLed g_led_l;
PifDutyMotor g_motor;
PifTimerManager g_timer_1ms;

static int CmdDutyMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "mt", CmdDutyMotorTest, "Motor Test", NULL },

	{ NULL, NULL, NULL, NULL }
};

typedef struct {
	uint16_t usDuty;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 128 };


static int CmdDutyMotorTest(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Duty: %d\n", g_motor._current_duty);
		pifLog_Printf(LT_NONE, "  Direction: %d\n", g_motor._direction);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (!strcmp(argv[0], "duty")) {
			int value = atoi(argv[1]);
			if (value > 0 && value < 256) {
				pifDutyMotor_SetDuty(&g_motor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "dir")) {
			int value = atoi(argv[1]);
			if (value == 0 || value == 1) {
				pifDutyMotor_SetDirection(&g_motor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "stop")) {
			pifDutyMotor_BreakRelease(&g_motor, 0);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "break")) {
			pifDutyMotor_BreakRelease(&g_motor, 1000);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "start")) {
			pifDutyMotor_Start(&g_motor, s_stDutyMotorTest.usDuty);
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

BOOL appSetup()
{
    if (!pifLog_UseCommand(32, c_psCmdTable, "\nDebug> ")) return FALSE;	// 32bytes

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;					// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
