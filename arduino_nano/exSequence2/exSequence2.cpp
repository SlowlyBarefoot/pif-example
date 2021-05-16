// Do not remove the include below
#include <MsTimer2.h>

#include "exSequence2.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_PUSH_SWITCH_1		5
#define PIN_PUSH_SWITCH_2		6


static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
} s_stSequenceTest[SEQUENCE_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW }
};


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedLState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

void actLedRGBState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(s_stSequenceTest[0].ucPinLed, unState & 1);
	digitalWrite(s_stSequenceTest[1].ucPinLed, (unState >> 1) & 1);
}

uint16_t actPushSwitchAcquire(PIF_usId usPifId)
{
	return !digitalRead(s_stSequenceTest[usPifId - PIF_ID_SWITCH].ucPinSwitch);
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
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
