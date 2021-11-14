#include "appMain.h"
#include "exLed1.h"

#include "pif_led.h"
#include "pif_log.h"


PifTimerManager *g_pstTimer1ms = NULL;

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
	PifComm *pstCommLog;

    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 2);				// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;							// 500ms

    s_pstLedRGB = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedRGBState);
    if (!s_pstLedRGB) return;
    if (!pifLed_AttachBlink(s_pstLedRGB, 100)) return;							// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLed, NULL, TRUE)) return;	// 100ms

    pifLed_BlinkOn(s_pstLedL, 0);

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(g_pstTimer1ms));
}
