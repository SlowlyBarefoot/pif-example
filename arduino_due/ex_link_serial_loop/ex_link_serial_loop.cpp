// Do not remove the include below
#include "ex_link_serial_loop.h"
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
