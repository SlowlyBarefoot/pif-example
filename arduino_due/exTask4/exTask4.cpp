// Do not remove the include below
#include "exTask4.h"

#include "pifTask.h"


#define PIN_LED_L				13

#define TASK_COUNT              1


static void _LedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	pif_Init();

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriod(500, _LedToggle, NULL)) return;	// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
