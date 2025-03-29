/**
 * Pulse 1개는 1ms Timer에 연결하고 또 1개는 100us Timer에 연결한다.
 * 2 Timer 모두 500ms 마다 이벤트를 발생시키고 이벤트에서 LED를 깜박가리게 한다.
 * 여기서 Scale 사용법을 알 수 있다.
 *
 * One pulse is connected to the 1 ms timer, and the other is connected to the 100us timer.
 * 2 Both Timers generate an event every 500 ms and flash the LED at the event.
 * Here you can see how to use the scale.
 */

// Do not remove the include below
#include <DueTimer.h>

#include "exTimer2.h"

#include "core/pif_timer.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25

#define TASK_SIZE				2
#define TIMER_1MS_SIZE			1
#define TIMER_100US_SIZE		1


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

//The setup function is called once at startup of the sketch
void setup()
{
	PifTimer *pstTimer1ms;
	PifTimer *pstTimer100us;

	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	Timer3.attachInterrupt(timer_100us).start(100);

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

    if (!pifTimerManager_Init(&g_timer_100us, PIF_ID_AUTO, 100, TIMER_100US_SIZE)) return;	// 100us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedRedToggle, NULL);
    if (!pifTimer_Start(pstTimer1ms, 5)) return;											// 5 * 1ms = 5ms

    pstTimer100us = pifTimerManager_Add(&g_timer_100us, TT_REPEAT);
    if (!pstTimer100us) return;
    pifTimer_AttachEvtFinish(pstTimer100us, evtLedYellowToggle, NULL);
    if (!pifTimer_Start(pstTimer100us, 5)) return;											// 5 * 100us = 500us
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
