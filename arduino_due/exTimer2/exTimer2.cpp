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
#include "appMain.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25


extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(g_pstTimer1ms);
		return 0;
	}
}

static void timer_100us()
{
	pifTimerManager_sigTick(g_pstTimer100us);
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
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	Timer3.attachInterrupt(timer_100us).start(100);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
