// Do not remove the include below
#include "exLed1.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState)
{
	(void)usPifId;
	(void)ucIndex;

	digitalWrite(PIN_LED_L, swState);
}

void actLedRGBState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState)
{
	(void)usPifId;

	switch (ucIndex) {
	case 0:
		digitalWrite(PIN_LED_RED, swState);
		break;

	case 1:
		digitalWrite(PIN_LED_YELLOW, swState);
		break;

	case 2:
		digitalWrite(PIN_LED_GREEN, swState);
		break;
	}
	pifLog_Printf(LT_enInfo, "RGB:%u I=%u S:%u", __LINE__, ucIndex, swState);
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
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
