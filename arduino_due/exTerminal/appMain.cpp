#include <stdlib.h>

#include "appMain.h"
#include "exTerminal.h"

#include "pif_led.h"
#include "pif_log.h"


PifTimerManager g_timer_1ms;
PifComm g_comm_log;

static PifLed s_led_l;

static BOOL bBlink = TRUE;
static int nPeriod = 500;

static int _CmdLedControl(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "led", _CmdLedControl, "\nLED Control" },

	{ NULL, NULL, NULL }
};


static int _CmdLedControl(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "\n  Blink=%u", bBlink);
		pifLog_Printf(LT_NONE, "\n  Period=%u", nPeriod);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		switch (argv[1][0]) {
		case 'F':
		case 'f':
		    pifLed_BlinkOff(&s_led_l, 0);
		    bBlink = FALSE;
			break;

		case 'T':
		case 't':
		    pifLed_BlinkOn(&s_led_l, 0);
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

void appSetup(PifActTimer1us act_timer1us)
{
    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
    g_comm_log.act_receive_data = actLogReceiveData;
    g_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&g_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, nPeriod)) return;
    pifLed_BlinkOn(&s_led_l, 0);
    bBlink = TRUE;

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
