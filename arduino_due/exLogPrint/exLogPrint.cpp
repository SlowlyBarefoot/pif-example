/**
 * exTimer1과 동일하나 LED 깜박일때마다 Log를 출력한다.
 *
 * It is the same as exTimer1, but outputs Log whenever LED flashes.
 */

// Do not remove the include below
#include "exLogPrint.h"
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

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(g_pstTimer1ms);
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
