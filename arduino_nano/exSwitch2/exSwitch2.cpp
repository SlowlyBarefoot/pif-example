// Do not remove the include below
#include <MsTimer2.h>

#include "exSwitch2.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_PUSH_SWITCH			5
#define PIN_TILT_SWITCH			6


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

void evtSwitchAcquire(void *pvIssuer)
{
	(void)pvIssuer;

	pifSensorSwitch_sigData(g_pstPushSwitch, !digitalRead(PIN_PUSH_SWITCH));
	pifSensorSwitch_sigData(g_pstTiltSwitch, digitalRead(PIN_TILT_SWITCH));
}

void evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	digitalWrite(PIN_LED_RED, usLevel);
}

void evtTiltSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	digitalWrite(PIN_LED_YELLOW, usLevel);
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
}
