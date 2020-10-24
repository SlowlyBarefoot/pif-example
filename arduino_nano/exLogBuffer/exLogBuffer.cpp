// Do not remove the include below
#include <MsTimer2.h>

#include "exLogBuffer.h"

#include "pifLog.h"
#include "pifPulse.h"
#include "pifTask.h"


#define PIN_NANO_LED			13

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              1

#define LOG_BUFFER_SIZE			0x200

static PIF_stPulse *g_pstTimer1ms = NULL;
static char s_acLog[LOG_BUFFER_SIZE];


void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(g_pstTimer1ms);
}

static void log_print(char *pcString)
{
	Serial.print(pcString);
}

static void led_toggle(void *pvIssuer)
{
	static BOOL sw = LOW;
	static int count = 9;

	(void)pvIssuer;

	digitalWrite(PIN_NANO_LED, sw);
	sw ^= 1;

	pifLog_Printf(LT_enInfo, "%d", sw);

	if (count) count--;
	else {
	    pifLog_PrintInBuffer();
	    count = 9;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_stPulseItem *pstTimer1ms;

	pinMode(PIN_NANO_LED, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

	pif_Init();

	pifLog_Init();
	pifLog_AttachActPrint(log_print);
    pifLog_InitBufferShare(LOG_BUFFER_SIZE, s_acLog);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!g_pstTimer1ms) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) return;

    pstTimer1ms = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, led_toggle, NULL);
    pifPulse_StartItem(pstTimer1ms, 1000);	// 1000 * 1ms = 1sec

    pifLog_Disable();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}