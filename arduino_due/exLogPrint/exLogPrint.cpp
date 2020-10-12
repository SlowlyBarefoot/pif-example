/**
 * exTimer1과 동일하나 LED 깜박일때마다 Log를 출력한다.
 *
 * It is the same as exTimer1, but outputs Log whenever LED flashes.
 */

// Do not remove the include below
#include "exLogPrint.h"

#include "pifLog.h"
#include "pifPulse.h"
#include "pifTask.h"


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

static void log_print(char *pcString)
{
	Serial.print(pcString);
}

static void led_toggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_DUE_LED, sw);
	sw ^= 1;

	pifLog_Printf(LT_enInfo, "%d", sw);
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_stPulseItem *pstTimer1ms;

	pinMode(PIN_DUE_LED, OUTPUT);

	Serial.begin(115200);

	pif_Init();

	pifLog_Init();
	pifLog_AttachActPrint(log_print);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PULSE_ITEM_COUNT, 1);
    if (!g_pstTimer1ms) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) return;

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, led_toggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 1000);	// 1000ms = 1sec
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}