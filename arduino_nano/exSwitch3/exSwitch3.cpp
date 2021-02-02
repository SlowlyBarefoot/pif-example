// Do not remove the include below
#include <MsTimer2.h>

#include "exSwitch3.h"
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

void actLedState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState)
{
	(void)usPifId;

	switch (ucIndex) {
	case 0:
		digitalWrite(PIN_LED_L, swState);
		break;

	case 1:
		digitalWrite(PIN_LED_RED, swState);
		break;

	case 2:
		digitalWrite(PIN_LED_YELLOW, swState);
		break;
	}
}

void evtSwitchAcquire(void *pvIssuer)
{
	(void)pvIssuer;

	pifSwitch_sigData(g_pstPushSwitch, digitalRead(PIN_PUSH_SWITCH));
	pifSwitch_sigData(g_pstTiltSwitch, digitalRead(PIN_TILT_SWITCH));
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
