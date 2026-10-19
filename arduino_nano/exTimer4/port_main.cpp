#include "exTimer4.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_LED_GREEN			4


static void actLedRGBState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_RED, unState & 1);
	digitalWrite(PIN_LED_YELLOW, (unState >> 1) & 1);
	digitalWrite(PIN_LED_GREEN, (unState >> 2) & 1);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

BOOL portSetup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;		// 1000us

    if (!pifLed_Init(&g_led_rgb, PIF_ID_AUTO, 3, actLedRGBState)) goto fail;
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
