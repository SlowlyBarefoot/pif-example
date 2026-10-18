// Do not remove the include below
#include "exCollectSignal2.h"
#include "app_port.h"

//The setup function is called once at startup of the sketch
void setup()
{
	if (!portSetup()) return;
	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
