// Do not remove the include below
#include <MsTimer2.h>

#include "exTask4.h"

#include "pifTask.h"


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3

#define TASK_COUNT              3


static void _taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

static void _taskLedRedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_RED, sw);
	sw ^= 1;
}

static void _taskLedYellowToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_YELLOW, sw);
	sw ^= 1;
}

void sysTickHook()
{
	pif_sigTimer1ms();
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	pif_Init();

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriodMs(500, _taskLedToggle, NULL)) return;		// 500ms
    if (!pifTask_AddRatio(100, _taskLedRedToggle, NULL)) return;		// 100%
    if (!pifTask_AddPeriodUs(500, _taskLedYellowToggle, NULL)) return;	// 500us
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
