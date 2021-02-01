// Do not remove the include below
#include "exSwitch2.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_PUSH_SWITCH			29
#define PIN_TILT_SWITCH			31


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

void evtSwitchAcquire(void *pvIssuer)
{
	(void)pvIssuer;

	pifSwitch_sigData(g_pstPushSwitch, digitalRead(PIN_PUSH_SWITCH));
	pifSwitch_sigData(g_pstTiltSwitch, digitalRead(PIN_TILT_SWITCH));
}

void evtPushSwitchChange(PIF_usId usPifId, SWITCH swState, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	digitalWrite(PIN_LED_RED, swState);
}

void evtTiltSwitchChange(PIF_usId usPifId, SWITCH swState, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	digitalWrite(PIN_LED_YELLOW, swState);
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
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);
	pinMode(PIN_TILT_SWITCH, INPUT_PULLUP);

	Serial.begin(115200); //Doesn't matter speed

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
