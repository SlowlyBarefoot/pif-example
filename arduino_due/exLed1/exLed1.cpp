// Do not remove the include below
#include "exLed1.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27


uint16_t actLogSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

void actLedRGBState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_RED, unState & 1);
	digitalWrite(PIN_LED_YELLOW, (unState >> 1) & 1);
	digitalWrite(PIN_LED_GREEN, (unState >> 2) & 1);
	pifLog_Printf(LT_enInfo, "RGB:%u S:%xh", __LINE__, unState);
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifPulse_sigTick(g_pstTimer1ms);
		return 0;
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
}
