#include "appMain.h"
#include "exLed1.h"

#include "pifLed.h"
#include "pifLog.h"


#define LED_COUNT         		2
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define TASK_COUNT              2


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedL = NULL;
static PIF_stLed *s_pstLedRGB = NULL;


static uint16_t _taskLed(PIF_stTask *pstTask)
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
	pifLog_AttachActPrint(actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    s_pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;						// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    s_pstLedRGB = pifLed_Add(PIF_ID_AUTO, 3, actLedRGBState);
    if (!s_pstLedRGB) return;
    if (!pifLed_AttachBlink(s_pstLedRGB, 100)) return;						// 100ms

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%

    if (!pifTask_AddPeriodMs(100, _taskLed, NULL)) return;					// 100ms
}
