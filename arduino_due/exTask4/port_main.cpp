#include "exTask4.h"
#include "app_port.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25


static uint32_t taskLedToggle(PifTask *pstTask)
{
	static BOOL sw = FALSE;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
	return 0;
}

static uint32_t taskLedRedToggle(PifTask *pstTask)
{
	uint32_t rtn = 0;
	static int nCount = 0, nToggle = 0;
	static BOOL sw = FALSE;

    if (!nCount) {
    	digitalWrite(PIN_LED_RED, sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			rtn = pstTask->_default_period * 1000;
		}

		nCount = 99;
    }
    else nCount--;
    return rtn;
}

static uint32_t taskLedYellowToggle(PifTask *pstTask)
{
	uint32_t rtn = 0;
	static int nCount = 0, nToggle = 0;
	static BOOL sw = FALSE;

    if (!nCount) {
    	digitalWrite(PIN_LED_YELLOW, sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			rtn = pstTask->_default_period * 1000;
		}

		nCount = 99;
    }
    else nCount--;
    return rtn;
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

    if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return FALSE;			// 500ms
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 100, taskLedRedToggle, NULL, TRUE)) return FALSE;			// 100us
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 200, taskLedYellowToggle, NULL, TRUE)) return FALSE;		// 200us
    return TRUE;
}
