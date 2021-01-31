#include "appMain.h"
#include "exTaskDelay.h"

#include "pifLed.h"
#include "pifLog.h"


#define LED_COUNT         		2
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define TASK_COUNT              2


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedL = NULL;
static PIF_stLed *s_pstLedRGB = NULL;


static void _taskLed(PIF_stTask *pstTask)
{
	if (pifTask_FirstDelay(pstTask)) {
		pifLed_On(s_pstLedRGB, 0);
	}
	if (pifTask_NextDelay(pstTask)) {
		//
	}
	if (pifTask_NextDelay(pstTask)) {
		pifTask_SetDelay(pstTask, 500);		// 500ms
	}
	if (pifTask_NextDelay(pstTask)) {
		//
	}
	if (pifTask_LastDelay(pstTask)) {
		pifLed_Off(s_pstLedRGB, 0);
	}
}

void appSetup()
{
    pif_Init();

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
