/**
 * exTask2는 모든 Task를 동시에 시작하지만 이 예제에서는 각 Task를 순차적으로 동작시킨다.
 *
 * exTask2 starts all tasks simultaneously, but in this example, each task is operated sequentially.
 */

// Do not remove the include below
#include "exTask3.h"
#include "appMain.h"

#include "pifTask.h"


#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_LED_GREEN			4


void actLedRed(SWITCH sw)
{
	digitalWrite(PIN_LED_RED, sw);
}

void actLedYellow(SWITCH sw)
{
	digitalWrite(PIN_LED_YELLOW, sw);
}

void actLedGreen(SWITCH sw)
{
	digitalWrite(PIN_LED_GREEN, sw);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
