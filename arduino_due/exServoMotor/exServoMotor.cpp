// Do not remove the include below
#include <DueTimer.h>

#include "exServoMotor.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PWM					9


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

void actPulsePwm(SWITCH swValue)
{
	digitalWrite(PIN_PWM, swValue);
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

static void timer_100us()
{
	pifTimerManager_sigTick(&g_timer_100us);
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
}
