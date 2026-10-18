/**
 * exTask2는 모든 Task를 동시에 시작하지만 이 예제에서는 각 Task를 순차적으로 동작시킨다.
 *
 * exTask2 starts all tasks simultaneously, but in this example, each task is operated sequentially.
 */

// Do not remove the include below
#include "exTask3.h"
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
