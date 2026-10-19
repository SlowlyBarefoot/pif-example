#include "exTask1.h"
#include "app_port.h"


#define PIN_LED_L				13


static uint32_t _taskLedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_L, sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

BOOL portSetup()
{
	pinMode(PIN_LED_L, OUTPUT);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) goto fail;

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 50, _taskLedToggle, NULL, TRUE)) goto fail;	// 50us
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
