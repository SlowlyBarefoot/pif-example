/**
 * LED를 깜박이게 하는 Task를 생성한다.
 *
 * Produces a task that blinks the LED.
 */

// Do not remove the include below
#include "exTask1.h"

#include "core/pif_task.h"


#define PIN_LED_L				13

#define TASK_SIZE				1


static uint32_t _taskLedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	digitalWrite(PIN_LED_L, sw);
		sw ^= 1;

		nCount = 999;
    }
    else nCount--;
    return 0;
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

	if (!pifTaskManager_Add(TM_PERIOD, 50, _taskLedToggle, NULL, TRUE)) return;	// 50us
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
