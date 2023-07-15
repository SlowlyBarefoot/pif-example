#include <stdlib.h>

#include "app_main.h"
#include "main.h"

#include "core/pif_log.h"
#include "display/pif_led.h"


PifTimerManager g_timer_1ms;
PifUart g_uart_log;

static PifLed s_led_l;

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
		    pifLed_SBlinkOff(&s_led_l, 1 << 0, OFF);
		    bBlink = FALSE;
			break;

		case 'T':
		case 't':
		    pifLed_SBlinkOn(&s_led_l, 1 << 0);
		    bBlink = TRUE;
			break;

		default:
			nPeriod = atoi(argv[0]);
			if (nPeriod) {
				pifLed_ChangeBlinkPeriod(&s_led_l, nPeriod);
			}
			else return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

void _evtLogControlChar(char ch)
{
	pifLog_Printf(LT_INFO, "Contorl Char = %x\n", ch);
}

void appSetup()
{
    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;		// 1000us

	if (!pifUart_Init(&g_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_log, TM_PERIOD_MS, 1, NULL)) return;		// 1ms
	if (!pifUart_AllocRxBuffer(&g_uart_log, 64, 100)) return;					// 100%
	if (!pifUart_AllocTxBuffer(&g_uart_log, 128)) return;
	g_uart_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachUart(&g_uart_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;
    pifLog_AttachEvent(_evtLogControlChar);

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, nPeriod)) return;

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
