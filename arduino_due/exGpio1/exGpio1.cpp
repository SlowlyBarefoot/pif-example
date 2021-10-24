// Do not remove the include below
#include "exGpio1.h"
#include "appMain.h"

#include "pif_log.h"
#include "pif_task.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_PUSH_SWITCH			29
#define PIN_TILT_SWITCH			31


uint16_t actLogSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

void actGpioLedL(PifId usPifId, uint8_t ucState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, ucState);
}

void actGpioLedRG(PifId usPifId, uint8_t ucState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_RED, ucState & 1);
	digitalWrite(PIN_LED_YELLOW, (ucState >> 1) & 1);
}

uint8_t actGpioSwitch(PifId usPifId)
{
	(void)usPifId;

	return digitalRead(PIN_PUSH_SWITCH) + (digitalRead(PIN_TILT_SWITCH) << 1);
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);
	pinMode(PIN_TILT_SWITCH, INPUT_PULLUP);

	Serial.begin(115200); //Doesn't matter speed

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
