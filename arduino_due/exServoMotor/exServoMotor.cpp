// Do not remove the include below
#include <DueTimer.h>

#include "exServoMotor.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PWM					9


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

void actPulsePwm(SWITCH swValue)
{
	digitalWrite(PIN_PWM, swValue);
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(g_pstTimer1ms);
	}
}

static void timer_100us()
{
	pifPulse_sigTick(g_pstTimer100us);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PWM, OUTPUT);

	Timer3.attachInterrupt(timer_100us).start(100);

	Serial.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
