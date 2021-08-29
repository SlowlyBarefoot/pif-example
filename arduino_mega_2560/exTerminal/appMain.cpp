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

void appSetup(PIF_actTimer1us actTimer1us)
{
    pif_Init(actTimer1us);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_AddRatio(100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;	// 100%

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    g_pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifTask_AddPeriodMs(1, pifComm_Task, g_pstCommLog, TRUE)) return;	// 1ms
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

    s_pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, nPeriod)) return;
    pifLed_BlinkOn(s_pstLedL, 0);
    bBlink = TRUE;

    if (!pifTask_AddPeriodMs(20, pifLog_Task, NULL, TRUE)) return;			// 20ms
}
