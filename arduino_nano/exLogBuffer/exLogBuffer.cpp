// Do not remove the include below
#include <MsTimer2.h>

#include "exLogBuffer.h"
#include "appMain.h"


#define PIN_LED_L				13


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedL(SWITCH sw)
{
	digitalWrite(PIN_LED_L, sw);
}

static void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(g_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
