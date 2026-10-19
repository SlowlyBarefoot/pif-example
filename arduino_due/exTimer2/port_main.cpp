#include "exTimer2.h"
#include "app_port.h"

#include <DueTimer.h>


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25


PifTimerManager g_timer_1ms;
PifTimerManager g_timer_100us;


extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

static void timer_100us()
{
	pifTimerManager_sigTick(&g_timer_100us);
}

void evtLedRedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_RED, sw);
	sw ^= 1;
}

void evtLedYellowToggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_YELLOW, sw);
	sw ^= 1;
}

BOOL portSetup()
{
	PifTimer *pstTimer1ms;
	PifTimer *pstTimer100us;

	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	Timer3.attachInterrupt(timer_100us).start(100);

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 2)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;		// 1000us

    if (!pifTimerManager_Init(&g_timer_100us, PIF_ID_AUTO, 100, TIMER_100US_SIZE)) goto fail;	// 100us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) goto fail;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedRedToggle, NULL, FALSE);
    if (!pifTimer_Start(pstTimer1ms, 5)) goto fail;												// 5 * 1ms = 5ms

    pstTimer100us = pifTimerManager_Add(&g_timer_100us, TT_REPEAT);
    if (!pstTimer100us) goto fail;
    pifTimer_AttachEvtFinish(pstTimer100us, evtLedYellowToggle, NULL, FALSE);
    if (!pifTimer_Start(pstTimer100us, 5)) goto fail;											// 5 * 100us = 500us
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
