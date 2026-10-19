#include "exTask4.h"
#include "app_port.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25


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

BOOL portSetup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) goto fail;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) goto fail;			// 500ms
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 100, taskLedRedToggle, NULL, TRUE)) goto fail;			// 100us
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 200, taskLedYellowToggle, NULL, TRUE)) goto fail;		// 200us
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
