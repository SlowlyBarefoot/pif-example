// Do not remove the include below
#include <DueTimer.h>

#include "pifPulse.h"

#include "exTimer2.h"


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
// Add your initialization code here
	PIF_unPulseIndex unTimer1msIndex;
	PIF_unPulseIndex unTimer100usIndex;

	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	Timer3.attachInterrupt(timer_100us).start(100);

    pif_Init();

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PULSE_ITEM_COUNT, 1);		// 1ms : 1ms
    if (!g_pstTimer1ms) return;
    g_pstTimer100us = pifPulse_Add(PULSE_ITEM_COUNT, 100);	// 100us : 1us
    if (!g_pstTimer100us) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) return;

    unTimer1msIndex = pifPulse_AddItem(g_pstTimer1ms, TT_enRepeat);
    if (unTimer1msIndex == PIF_PULSE_INDEX_NULL) return;
    pifPulse_AttachEvtFinish(g_pstTimer1ms, unTimer1msIndex, led_red_toggle, NULL);
    pifPulse_StartItem(g_pstTimer1ms, unTimer1msIndex, 500);	// 500ms = 0.5sec

    unTimer100usIndex = pifPulse_AddItem(g_pstTimer100us, TT_enRepeat);
    if (unTimer100usIndex == PIF_PULSE_INDEX_NULL) return;
    pifPulse_AttachEvtFinish(g_pstTimer100us, unTimer100usIndex, led_yellow_toggle, NULL);
    pifPulse_StartItem(g_pstTimer100us, unTimer100usIndex, 500000);	// 500000us = 0.5sec
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
    pif_Loop();

    pifTask_Loop();
}
