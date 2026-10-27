#include "exTask1.h"
#include "app_port.h"


#define PIN_LED_L				13


static uint32_t _taskLedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = FALSE;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_L, sw);
		sw ^= 1;

		nCount = 499;
    }
    else nCount--;
    return 0;
}

BOOL portSetup()
{
	pinMode(PIN_LED_L, OUTPUT);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskLedToggle, NULL, TRUE)) return FALSE;	// 1ms
	return TRUE;
}
