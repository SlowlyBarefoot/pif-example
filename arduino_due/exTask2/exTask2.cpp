/**
 * LED를 깜박이게 하는 Task 3개를 생성한다. 각 Task의 Ratio를 다르게 설정한다.
 *
 * Produce three tasks that make the LED flash. Set the Ratio for each task differently.
 */

// Do not remove the include below
#include "exTask2.h"

#include "core/pif_task.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27

#define TASK_SIZE				3


static uint16_t _taskLedRedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_RED, sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

static uint16_t _taskLedYellowToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_YELLOW, sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

static uint16_t _taskLedGreenToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_GREEN, sw);
		sw ^= 1;

		nCount = 9999;
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

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTaskManager_Add(TM_RATIO, 30, _taskLedRedToggle, NULL, TRUE)) return;		// 30%
    if (!pifTaskManager_Add(TM_RATIO, 60, _taskLedYellowToggle, NULL, TRUE)) return;	// 60%
    if (!pifTaskManager_Add(TM_ALWAYS, 100, _taskLedGreenToggle, NULL, TRUE)) return;	// 100%
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
