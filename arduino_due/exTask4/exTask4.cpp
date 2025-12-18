// Do not remove the include below
#include "exTask4.h"

#include "core/pif_task_manager.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25

#define TASK_SIZE				3


static uint32_t taskLedToggle(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
	return 0;
}

static uint32_t taskLedRedToggle(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_RED, sw);
	sw ^= 1;
	return 0;
}

static uint32_t taskLedYellowToggle(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_YELLOW, sw);
	sw ^= 1;
	return 0;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return;			// 500ms
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_ALWAYS, 100, taskLedRedToggle, NULL, TRUE)) return;			// 100%
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 200, taskLedYellowToggle, NULL, TRUE)) return;		// 200us
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
