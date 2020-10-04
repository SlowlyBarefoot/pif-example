// Do not remove the include below
#include "pifPulse.h"

#include "exTimer1.h"


#define PIN_DUE_LED				13

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              1


static PIF_stPulse *g_pstTimer1ms = NULL;


extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(g_pstTimer1ms);
	}
}

static void led_toggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_DUE_LED, sw);
	sw ^= 1;
}

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here
	PIF_unPulseIndex unTimer1msIndex;

	pinMode(PIN_DUE_LED, OUTPUT);

	pif_Init();

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PULSE_ITEM_COUNT, 1);
    if (!g_pstTimer1ms) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) return;

    unTimer1msIndex = pifPulse_AddItem(g_pstTimer1ms, TT_enRepeat);
    if (unTimer1msIndex == PIF_PULSE_INDEX_NULL) return;
    pifPulse_AttachEvtFinish(g_pstTimer1ms, unTimer1msIndex, led_toggle, NULL);
    pifPulse_StartItem(g_pstTimer1ms, unTimer1msIndex, 500);	// 500ms = 0.5sec
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
    pif_Loop();

    pifTask_Loop();
}
