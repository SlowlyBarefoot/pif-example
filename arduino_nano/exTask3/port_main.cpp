#include "exTask3.h"
#include "app_port.h"


#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_LED_GREEN			4


static PifTask *s_pstTask[3];


static uint32_t _taskLedRedToggle(PifTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	digitalWrite(PIN_LED_RED, sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pifTask_Pause(pstTask);
			pifTask_Restart(s_pstTask[1]);
		}

		nCount = 99;
    }
    else nCount--;
    return 0;
}

static uint32_t _taskLedYellowToggle(PifTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	digitalWrite(PIN_LED_YELLOW, sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pifTask_Pause(pstTask);
			pifTask_Restart(s_pstTask[2]);
		}

		nCount = 99;
    }
    else nCount--;
    return 0;
}

static uint32_t _taskLedGreenToggle(PifTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	digitalWrite(PIN_LED_GREEN, sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pifTask_Pause(pstTask);
			pifTask_Restart(s_pstTask[0]);
		}

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

    s_pstTask[0] = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 200, _taskLedRedToggle, NULL, TRUE);		// 200us
    if (!s_pstTask[0]) goto fail;

    s_pstTask[1] = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskLedYellowToggle, NULL, FALSE);	// 1ms
    if (!s_pstTask[1]) goto fail;

    s_pstTask[2] = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500, _taskLedGreenToggle, NULL, FALSE);	// 500us
    if (!s_pstTask[2]) goto fail;
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
