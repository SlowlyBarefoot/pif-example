#include "app_port.h"

#include <stdlib.h>


PifLed g_led_l;
PifRtt g_rtt;

static int _CmdBlinkControl(int argc, char *argv[]);
static int _CmdRtt(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "blink", _CmdBlinkControl, "Blink Control", NULL },
	{ "rtt", _CmdRtt, "RTT state", NULL },

	{ NULL, NULL, NULL, NULL }
};


static int _CmdBlinkControl(int argc, char *argv[])
{
	int period;

	if (argc == 0) return PIF_LOG_CMD_TOO_FEW_ARGS;
	switch (argv[0][0]) {
	case 'F':
	case 'f':
		pifLed_SBlinkOff(&g_led_l, 1 << 0, OFF);
		break;

	case 'T':
	case 't':
		pifLed_SBlinkOn(&g_led_l, 1 << 0);
		break;

	default:
		period = atoi(argv[0]);
		if (!period) return PIF_LOG_CMD_INVALID_ARG;
		pifLed_ChangeBlinkPeriod(&g_led_l, period);
	}
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdRtt(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pifLog_Printf(LT_NONE, "  Channel=%u TX=%u bytes waiting\n", g_rtt._index,
			pifUart_GetFillSizeOfTxBuffer(&g_rtt.parent));
	return PIF_LOG_CMD_NO_ERROR;
}

BOOL appSetup()
{
    if (!pifLog_UseCommand(32, c_psCmdTable, "\nRTT> ")) return FALSE;	// 32bytes
    pifLog_SetEcho(FALSE);												// RTT viewers show what is typed

	if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;				// 500ms
	g_led_l._p_task->name = "LedL";
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***              ex_rtt              ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);
	return TRUE;
}
