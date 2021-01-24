/**
 * exTask2는 모든 Task를 동시에 시작하지만 이 예제에서는 각 Task를 순차적으로 동작시킨다.
 *
 * exTask2 starts all tasks simultaneously, but in this example, each task is operated sequentially.
 */

// Do not remove the include below
#include "exTask3.h"

#include "pifTask.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27

#define TASK_COUNT              3


static PIF_stTask *s_pstTask[TASK_COUNT];


static void led_red_toggle(PIF_stTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
		digitalWrite(PIN_LED_RED, sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->bPause = TRUE;
			s_pstTask[1]->bPause = FALSE;
		}

		nCount = 9999;
    }
    else nCount--;
}

static void led_yellow_toggle(PIF_stTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
		digitalWrite(PIN_LED_YELLOW, sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->bPause = TRUE;
			s_pstTask[2]->bPause = FALSE;
		}

		nCount = 9999;
    }
    else nCount--;
}

static void led_green_toggle(PIF_stTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
		digitalWrite(PIN_LED_GREEN, sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->bPause = TRUE;
			s_pstTask[0]->bPause = FALSE;
		}

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

    s_pstTask[0] = pifTask_AddRatio(30, led_red_toggle, NULL);		// 30%
    if (!s_pstTask[0]) return;

    s_pstTask[1] = pifTask_AddRatio(60, led_yellow_toggle, NULL);	// 60%
    if (!s_pstTask[1]) return;
    s_pstTask[1]->bPause = TRUE;

    s_pstTask[2] = pifTask_AddRatio(100, led_green_toggle, NULL);	// 100%
    if (!s_pstTask[2]) return;
    s_pstTask[2]->bPause = TRUE;
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
