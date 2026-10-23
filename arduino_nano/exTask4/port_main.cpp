#include "exTask4.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3


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

static void sysTickHook()
{
	pif_sigTimer1ms();
}

BOOL portSetup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

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
