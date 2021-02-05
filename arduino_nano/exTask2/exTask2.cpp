/**
 * LED를 깜박이게 하는 Task 3개를 생성한다. 각 Task의 Ratio를 다르게 설정한다.
 *
 * Produce three tasks that make the LED flash. Set the Ratio for each task differently.
 */

// Do not remove the include below
#include "exTask2.h"
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

    pifTask_Loop();
}
