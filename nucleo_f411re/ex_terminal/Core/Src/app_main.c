#include <stdlib.h>

#include "app_main.h"
#include "main.h"

#include "pif_led.h"
#include "pif_log.h"


PifTimerManager g_timer_1ms;
PifComm g_comm_log;


static PifLed s_led_l;

static BOOL bBlink = TRUE;
static int nPeriod = 500;

static int _CmdLedControl(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "led", _CmdLedControl, "LED Control" },

	{ NULL, NULL, NULL }
};


static int _CmdLedControl(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "  Blink=%u\n", bBlink);
		pifLog_Printf(LT_NONE, "  Period=%u\n", nPeriod);
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

void appSetup()
{
    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	if (!pifComm_AllocRxBuffer(&g_comm_log, 64)) return;
	if (!pifComm_AllocTxBuffer(&g_comm_log, 128)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, nPeriod)) return;
    pifLed_SBlinkOn(&s_led_l, 1 << 0);
    bBlink = TRUE;

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
