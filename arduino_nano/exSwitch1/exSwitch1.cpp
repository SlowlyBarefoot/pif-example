// Do not remove the include below
#include <MsTimer2.h>

#include "exSwitch1.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_PUSH_SWITCH			5
#define PIN_TILT_SWITCH			6


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

SWITCH actPushSwitchAcquire(PIF_usId usPifId)
{
	(void)usPifId;

	return digitalRead(PIN_PUSH_SWITCH);
}

void evtPushSwitchChange(PIF_usId usPifId, SWITCH swState, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	digitalWrite(PIN_LED_RED, swState);
}

SWITCH actTiltSwitchAcquire(PIF_usId usPifId)
{
	(void)usPifId;

	return digitalRead(PIN_TILT_SWITCH);
}

void evtTiltSwitchChange(PIF_usId usPifId, SWITCH swState, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	digitalWrite(PIN_LED_YELLOW, swState);
}

void taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL swLed = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);
	pinMode(PIN_TILT_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
