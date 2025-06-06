/**
 * exTask2는 모든 Task를 동시에 시작하지만 이 예제에서는 각 Task를 순차적으로 동작시킨다.
 *
 * exTask2 starts all tasks simultaneously, but in this example, each task is operated sequentially.
 */

// Do not remove the include below
#include "exTask3.h"

#include "core/pif_task.h"


#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_LED_GREEN			4

#define TASK_SIZE				3


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
			pstTask->pause = TRUE;
			s_pstTask[1]->pause = FALSE;
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
			pstTask->pause = TRUE;
			s_pstTask[2]->pause = FALSE;
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
			pstTask->pause = TRUE;
			s_pstTask[0]->pause = FALSE;
		}

		nCount = 99;
    }
    else nCount--;
    return 0;
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    s_pstTask[0] = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 200, _taskLedRedToggle, NULL, TRUE);		// 200us
    if (!s_pstTask[0]) return;

    s_pstTask[1] = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskLedYellowToggle, NULL, FALSE);	// 1ms
    if (!s_pstTask[1]) return;

    s_pstTask[2] = pifTaskManager_Add(PIF_ID_AUTO, TM_ALWAYS, 0, _taskLedGreenToggle, NULL, FALSE);
    if (!s_pstTask[2]) return;
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
