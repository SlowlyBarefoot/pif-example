/**
 * LED를 깜박이게 하는 Task를 생성한다.
 *
 * Produces a task that blinks the LED.
 */

// Do not remove the include below
#include "exTask1.h"
#include "app_port.h"

//The setup function is called once at startup of the sketch
void setup()
{
	portSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
