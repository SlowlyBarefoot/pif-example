/**
 * LED를 깜박이게 하는 Task 3개를 생성한다. 각 Task의 Ratio를 다르게 설정한다.
 *
 * Produce three tasks that make the LED flash. Set the Ratio for each task differently.
 */

// Do not remove the include below
#include "exTask2.h"
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
