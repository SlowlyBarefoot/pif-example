/**
 * LED를 깜박이게 하는 Task 3개를 생성한다. 각 Task의 Ratio를 다르게 설정한다.
 *
 * Produce three tasks that make the LED flash. Set the Ratio for each task differently.
 */

// Do not remove the include below
#include "exTask2.h"

#include "core/pif_task_manager.h"


#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_LED_GREEN			4

#define TASK_SIZE				4


static uint32_t _taskLedRedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

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
	static BOOL sw = LOW;

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
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_GREEN, sw);
		sw ^= 1;

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

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500, _taskLedRedToggle, NULL, TRUE)) return;		// 500us
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 2000, _taskLedYellowToggle, NULL, TRUE)) return;	// 2ms
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_ALWAYS, 0, _taskLedGreenToggle, NULL, TRUE)) return;
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
