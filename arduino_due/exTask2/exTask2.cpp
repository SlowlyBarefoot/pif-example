/**
 * LED를 깜박이게 하는 Task 3개를 생성한다. 각 Task의 Ratio를 다르게 설정한다.
 *
 * Produce three tasks that make the LED flash. Set the Ratio for each task differently.
 */

// Do not remove the include below
#include "exTask2.h"

#include "pifTask.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27

#define TASK_COUNT              3


static void led_red_toggle(PIF_stTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

    if (!nCount) {
		digitalWrite(PIN_LED_RED, sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
}

static void led_yellow_toggle(PIF_stTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

    if (!nCount) {
		digitalWrite(PIN_LED_YELLOW, sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
}

static void led_green_toggle(PIF_stTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

    if (!nCount) {
		digitalWrite(PIN_LED_GREEN, sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	pif_Init();

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(30, led_red_toggle, NULL)) return;		// 30%
    if (!pifTask_AddRatio(60, led_yellow_toggle, NULL)) return;		// 60%
    if (!pifTask_AddRatio(100, led_green_toggle, NULL)) return;		// 100%
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
