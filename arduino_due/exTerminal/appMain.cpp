#include <stdlib.h>

#include "appMain.h"
#include "exTerminal.h"

#include "pif_led.h"
#include "pif_log.h"


PifPulse *g_pstTimer1ms = NULL;

static PifLed *s_pstLedL;

static BOOL bBlink = TRUE;
static int nPeriod = 500;

#ifdef __PIF_LOG_COMMAND__

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

#endif

void appSetup(PifActTimer1us act_timer1us)
{
	PifComm *pstCommLog;

    pif_Init(act_timer1us);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;	// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
#ifdef __PIF_LOG_COMMAND__
	pifComm_AttachActReceiveData(pstCommLog, actLogReceiveData);
#endif
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;
#ifdef __PIF_LOG_COMMAND__
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;
#endif

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, nPeriod)) return;
    pifLed_BlinkOn(s_pstLedL, 0);
    bBlink = TRUE;

#ifdef __PIF_LOG_COMMAND__
    if (!pifLog_AttachTask(TM_PERIOD_MS, 20, TRUE)) return;				// 20ms
#endif
}
