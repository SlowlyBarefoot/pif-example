#include "appMain.h"
#include "main.h"

#include "pifLed.h"
#include "pifLog.h"


PifPulse *g_pstTimer1ms = NULL;
PifComm *g_pstCommLog = NULL;

static PifLed *s_pstLedL = NULL;
static PifLed *s_pstLedRGB = NULL;


static uint16_t _taskLed(PifTask *pstTask)
{
	static uint16_t usStep = 0;
	static uint16_t usTimer = 0;

	(void)pstTask;

	if (!usTimer) {
		switch (usStep) {
		case 0:
			pifLed_EachOn(s_pstLedRGB, 0);
			usStep = 1;
			usTimer = 6;
			break;

		case 1:
			pifLed_EachOff(s_pstLedRGB, 0);
			pifLed_EachOn(s_pstLedRGB, 1);
			usStep = 2;
			usTimer = 6;
			break;

		case 2:
			pifLed_EachOff(s_pstLedRGB, 1);
			pifLed_EachOn(s_pstLedRGB, 2);
			usStep = 3;
			usTimer = 6;
			break;

		case 3:
			pifLed_EachOff(s_pstLedRGB, 2);
			pifLed_BlinkOn(s_pstLedRGB, 0);
			usStep = 4;
			usTimer = 6;
			break;

		case 4:
			pifLed_BlinkOff(s_pstLedRGB, 0);
			pifLed_BlinkOn(s_pstLedRGB, 1);
			usStep = 5;
			usTimer = 6;
			break;

		case 5:
			pifLed_BlinkOff(s_pstLedRGB, 1);
			pifLed_BlinkOn(s_pstLedRGB, 2);
			usStep = 6;
			usTimer = 6;
			break;

		case 6:
			pifLed_BlinkOn(s_pstLedRGB, 1);
			pifLed_BlinkOn(s_pstLedRGB, 2);
			usStep = 7;
			usTimer = 6;
			break;

		case 7:
			pifLed_BlinkOn(s_pstLedRGB, 0);
			pifLed_BlinkOn(s_pstLedRGB, 1);
			pifLed_BlinkOff(s_pstLedRGB, 2);
			usStep = 8;
			usTimer = 6;
			break;

		case 8:
			pifLed_BlinkOn(s_pstLedRGB, 0);
			pifLed_BlinkOn(s_pstLedRGB, 1);
			pifLed_BlinkOn(s_pstLedRGB, 2);
			usStep = 9;
			usTimer = 6;
			break;

		case 9:
			pifLed_BlinkOff(s_pstLedRGB, 0);
			pifLed_BlinkOff(s_pstLedRGB, 1);
			pifLed_BlinkOff(s_pstLedRGB, 2);
			usStep = 0;
			usTimer = 6;
			break;
		}
	}
	else {
		usTimer--;
	}
	return 0;
}

void appSetup()
{
    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);							// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;		// 100%

    g_pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifComm_AttachTask(g_pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;		// 1ms
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 64)) return;
	pifComm_AttachActStartTransfer(g_pstCommLog, actLogStartTransfer);

	if (!pifLog_AttachComm(g_pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    s_pstLedRGB = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedRGBState);
    if (!s_pstLedRGB) return;
    if (!pifLed_AttachBlink(s_pstLedRGB, 100)) return;							// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLed, NULL, TRUE)) return;	// 100ms
}