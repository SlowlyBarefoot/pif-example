#include <stdlib.h>

#include "appMain.h"
#include "exTerminal.h"

#include "core/pif_log.h"
#include "display/pif_led.h"


PifTimerManager g_timer_1ms;
PifComm g_comm_log;

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
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "  Blink=%u\n", bBlink);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		switch (argv[1][0]) {
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
			nPeriod = atoi(argv[1]);
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
	pifLog_Printf(LT_INFO, "Contorl Char = %xh\n", ch);
}

void appSetup(PifActTimer1us act_timer1us)
{
    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;		// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE, NULL)) return;	// 1ms
#ifdef USE_SERIAL
    g_comm_log.act_receive_data = actLogReceiveData;
    g_comm_log.act_send_data = actLogSendData;
#endif
#ifdef USE_USART
	if (!pifComm_AllocRxBuffer(&g_comm_log, 64, 100)) return;					// 100%
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;
#endif

	if (!pifLog_AttachComm(&g_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;
    pifLog_AttachEvent(_evtLogControlChar);

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, nPeriod)) return;

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
