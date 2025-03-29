/**
 * Pulse 1개를 생성후 이를 1ms Timer에 연결한다.
 * 이 Pulse로 200ms후 적색 LED를 토글시키고 500ms후 노란색 LED를 토클시키고
 * 1000ms후 녹색 LED를 토글시키는 이 과정을 반복한다.
 * 여기서 Pulse의 PT_enOnce 형식을 알 수 있다.
 *
 * Create one pulse and connect it to the 1 ms timer.
 * Repeat the process of toggling the red LED after 200 ms with this pulse,
 * toggling the yellow LED after 500 ms, and toggling the green LED after 1000 ms.
 * Here you can see the PT_enOnce format of Pulse.
 */

// Do not remove the include below
#include "exTimer3.h"
#include "appMain.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27

#define TASK_SIZE				1
#define TIMER_1MS_SIZE			3


static void actLedRGBState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_RED, unState & 1);
	digitalWrite(PIN_LED_YELLOW, (unState >> 1) & 1);
	digitalWrite(PIN_LED_GREEN, (unState >> 2) & 1);
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;	// 1000us

    if (!pifLed_Init(&g_led_rgb, PIF_ID_AUTO, &g_timer_1ms, 3, actLedRGBState)) return;

    if (!appSetup()) return;
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
