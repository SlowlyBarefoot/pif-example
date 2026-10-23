#include "exTimer1.h"
#include "app_port.h"


#define PIN_LED_L				13


extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = FALSE;

	(void)pvIssuer;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

BOOL portSetup()
{
	pinMode(PIN_LED_L, OUTPUT);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;		// 1000us

    g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_led) goto fail;
    pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL, FALSE);
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
