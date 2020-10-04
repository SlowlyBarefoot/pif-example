/**
 * LED를 깜박이게 하는 Task를 생성한다.
 *
 * Produces a task that blinks the LED.
 */

// Do not remove the include below
#include "exTask1.h"

#include "pifTask.h"


#define PIN_DUE_LED				13

#define TASK_COUNT              1


static void led_toggle(PIF_stTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

    if (!nCount) {
		digitalWrite(PIN_DUE_LED, sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_DUE_LED, OUTPUT);

	pif_Init();

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(50, led_toggle, NULL)) return;
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
