#include "exTask2.h"
#include "app_port.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27


static uint32_t _taskLedRedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = FALSE;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_RED, sw);
		sw ^= 1;

		nCount = 99;
    }
    else nCount--;
    return 0;
}

static uint32_t _taskLedYellowToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = FALSE;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_YELLOW, sw);
		sw ^= 1;

		nCount = 99;
    }
    else nCount--;
    return 0;
}

static uint32_t _taskLedGreenToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = FALSE;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_GREEN, sw);
		sw ^= 1;

		nCount = 99;
    }
    else nCount--;
    return 0;
}

BOOL portSetup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) goto fail;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500, _taskLedRedToggle, NULL, TRUE)) goto fail;		// 500us
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 2000, _taskLedYellowToggle, NULL, TRUE)) goto fail;	// 2ms
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskLedGreenToggle, NULL, TRUE)) goto fail;	// 1ms
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
