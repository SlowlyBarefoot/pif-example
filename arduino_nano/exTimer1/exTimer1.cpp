/**
 * Pulse 1개를 생성후 이를 1ms Timer에 연결한다. 500ms마다 이벤트를 발생시키고 이벤트에서 LED를 깜박가리게 한다.
 * 여기서 Pulse의 기본 기능과 PT_enRepeat 형식을 알수 있다.
 *
 * Create one pulse and connect it to the 1ms timer. Generate an event every 500ms and flash the LED in the event.
 * Here you can see the basic function of Pulse and the format PT_enRepeat.
 */

// Do not remove the include below
#include <MsTimer2.h>

#include "exTimer1.h"
#include "appMain.h"


#define PIN_LED_L				13

#define TASK_SIZE				1
#define TIMER_1MS_SIZE			1


static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

    g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_led) return;
    pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL);

    if (!appSetup()) return;
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
