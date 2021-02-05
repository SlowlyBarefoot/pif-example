/**
 * LED를 깜박이게 하는 Task를 생성한다.
 *
 * Produces a task that blinks the LED.
 */

// Do not remove the include below
#include "exTask1.h"
#include "appMain.h"

#include "pifTask.h"


#define PIN_LED_L				13


void actLedL(SWITCH sw)
{
	digitalWrite(PIN_LED_L, sw);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
