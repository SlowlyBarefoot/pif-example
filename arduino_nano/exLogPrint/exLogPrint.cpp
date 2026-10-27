/**
 * exTimer1과 동일하나 LED 깜박일때마다 Log를 출력한다.
 *
 * It is the same as exTimer1, but outputs Log whenever LED flashes.
 */
// Do not remove the include below
#include "exLogPrint.h"
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
