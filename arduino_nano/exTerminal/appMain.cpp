#include <stdlib.h>

#include "appMain.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;

static BOOL bBlink = FALSE;
static int nPeriod = 500;

static int _CmdBlinkControl(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "blink", _CmdBlinkControl, "Blink Control", NULL },

	{ NULL, NULL, NULL, NULL }
};


static int _CmdBlinkControl(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Blink=%u\n", bBlink);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		switch (argv[0][0]) {
		case 'F':
		case 'f':
		    pifLed_SBlinkOff(&g_led_l, 1 << 0, OFF);
		    bBlink = FALSE;
			break;

		case 'T':
		case 't':
		    pifLed_SBlinkOn(&g_led_l, 1 << 0);
		    bBlink = TRUE;
			break;

		default:
			nPeriod = atoi(argv[0]);
			if (nPeriod) {
				pifLed_ChangeBlinkPeriod(&g_led_l, nPeriod);
			}
			else return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtLogControlChar(char ch)
{
	pifLog_Printf(LT_INFO, "Contorl Char = %x\n", ch);
}

BOOL appSetup()
{
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return FALSE;
    pifLog_AttachEvent(_evtLogControlChar);

	if (!pifLed_AttachSBlink(&g_led_l, nPeriod)) return FALSE;
	pifLed_SBlinkOn(&g_led_l, 1 << 0);
	return TRUE;
}
