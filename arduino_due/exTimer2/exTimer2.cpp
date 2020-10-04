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
#include "exTimer2.h"

#include "pifPulse.h"

#include <DueTimer.h>


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25

#define PULSE_COUNT         	2
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              1


static PIF_stPulse *g_pstTimer1ms = NULL;
static PIF_stPulse *g_pstTimer100us = NULL;


extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(g_pstTimer1ms);
	}
}

static void timer_100us()
{
	pifPulse_sigTick(g_pstTimer100us);
}

static void led_red_toggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_RED, sw);
	sw ^= 1;
}

static void led_yellow_toggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_YELLOW, sw);
	sw ^= 1;
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_unPulseItemIndex unTimer1msIndex;
	PIF_unPulseItemIndex unTimer100usIndex;

	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	Timer3.attachInterrupt(timer_100us).start(100);

    pif_Init();

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PULSE_ITEM_COUNT, 1);		// unScale : 1ms / 1ms
    if (!g_pstTimer1ms) return;
    g_pstTimer100us = pifPulse_Add(PULSE_ITEM_COUNT, 100);	// unScale : 100us / 1us
    if (!g_pstTimer100us) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) return;

    unTimer1msIndex = pifPulse_AddItem(g_pstTimer1ms, TT_enRepeat);
    if (unTimer1msIndex == PIF_PULSE_INDEX_NULL) return;
    pifPulse_AttachEvtFinish(g_pstTimer1ms, unTimer1msIndex, led_red_toggle, NULL);
    if (!pifPulse_StartItem(g_pstTimer1ms, unTimer1msIndex, 500)) return;			// unPulse : 500ms = 0.5sec

    unTimer100usIndex = pifPulse_AddItem(g_pstTimer100us, TT_enRepeat);
    if (unTimer100usIndex == PIF_PULSE_INDEX_NULL) return;
    pifPulse_AttachEvtFinish(g_pstTimer100us, unTimer100usIndex, led_yellow_toggle, NULL);
    if (!pifPulse_StartItem(g_pstTimer100us, unTimer100usIndex, 500000)) return;	// unPulse : 500000us = 0.5sec
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
