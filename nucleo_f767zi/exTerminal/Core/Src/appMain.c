#include <stdlib.h>

#include "appMain.h"
#include "main.h"

#include "pif_led.h"
#include "pif_log.h"


PifPulse *g_pstTimer1ms = NULL;
PifComm *g_pstCommLog = NULL;


static PifLed *s_pstLedL;

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
		    pifLed_BlinkOff(s_pstLedL, 0);
		    bBlink = FALSE;
			break;

		case 'T':
		case 't':
		    pifLed_BlinkOn(s_pstLedL, 0);
		    bBlink = TRUE;
			break;

		default:
			nPeriod = atoi(argv[1]);
			if (nPeriod) {
				pifLed_ChangeBlinkPeriod(s_pstLedL, nPeriod);
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

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000, 1);					// 1000us
    if (!g_pstTimer1ms) return;

    g_pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifComm_AttachTask(g_pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	if (!pifComm_AllocRxBuffer(g_pstCommLog, 64)) return;
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 128)) return;
	g_pstCommLog->act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(g_pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, nPeriod)) return;
    pifLed_BlinkOn(s_pstLedL, 0);
    bBlink = TRUE;

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifPulse_Count(g_pstTimer1ms));
}
