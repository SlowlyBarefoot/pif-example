#include <stdlib.h>

#include "appMain.h"
#include "exTerminal.h"

#include "pifLed.h"
#include "pifLog.h"


#define COMM_COUNT         		1
#define LED_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              3


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedL;

static BOOL bBlink = TRUE;
static int nPeriod = 500;

#ifdef __PIF_LOG_COMMAND__

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

#endif

void appSetup(PIF_actTimer1us actTimer1us)
{
	PIF_stComm *pstCommLog;

    pif_Init(actTimer1us);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;	// 100%

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return;	// 1ms
#ifdef __PIF_LOG_COMMAND__
	pifComm_AttachActReceiveData(pstCommLog, actLogReceiveData);
#endif
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;
#ifdef __PIF_LOG_COMMAND__
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;
#endif

    s_pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, nPeriod)) return;
    pifLed_BlinkOn(s_pstLedL, 0);
    bBlink = TRUE;

#ifdef __PIF_LOG_COMMAND__
    if (!pifLog_AttachTask(TM_enPeriodMs, 20, TRUE)) return;				// 20ms
#endif
}
