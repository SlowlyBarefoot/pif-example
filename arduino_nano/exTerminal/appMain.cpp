#include <stdlib.h>

#include "appMain.h"
#include "exTerminal.h"

#include "pifLed.h"
#include "pifLog.h"


PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stComm *g_pstCommLog = NULL;


static PIF_stLed *s_pstLedL;

static BOOL bBlink = TRUE;
static int nPeriod = 500;

static int _CmdLedControl(int argc, char *argv[]);

const PIF_stLogCmdEntry c_psCmdTable[] = {
	{ "led", _CmdLedControl, "\nLED Control" },

	{ NULL, NULL, NULL }
};


static int _CmdLedControl(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_enNone, "\n  Blink=%u", bBlink);
		pifLog_Printf(LT_enNone, "\n  Period=%u", nPeriod);
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

void appSetup(PifActTimer1us act_timer1us)
{
    pif_Init(act_timer1us);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;	// 100%

    g_pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifComm_AttachTask(g_pstCommLog, TM_enPeriodMs, 1, TRUE)) return;	// 1ms
#ifdef USE_SERIAL
	pifComm_AttachActReceiveData(g_pstCommLog, actLogReceiveData);
	pifComm_AttachActSendData(g_pstCommLog, actLogSendData);
#endif
#ifdef USE_USART
	if (!pifComm_AllocRxBuffer(g_pstCommLog, 64)) return;
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 64)) return;
	pifComm_AttachActStartTransfer(g_pstCommLog, actLogStartTransfer);
#endif

	if (!pifLog_AttachComm(g_pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, nPeriod)) return;
    pifLed_BlinkOn(s_pstLedL, 0);
    bBlink = TRUE;

    if (!pifLog_AttachTask(TM_enPeriodMs, 20, TRUE)) return;				// 20ms
}
