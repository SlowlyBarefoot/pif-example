// Do not remove the include below
#include "exTask4.h"
#include "app_port.h"

//The setup function is called once at startup of the sketch
void setup()
{
	pif_Init((PifActTimer1us)micros, portSetup, NULL);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
