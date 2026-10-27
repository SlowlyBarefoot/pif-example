// Do not remove the include below
#include "exDotMatrixS_SR.h"
#include "app_port.h"

//The setup function is called once at startup of the sketch
void setup()
{
	pif_Init((PifActTimer1us)micros, portSetup, appSetup);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
