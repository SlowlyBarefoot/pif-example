// Do not remove the include below
#include <MsTimer2.h>

#include "exSensorThreshold2P.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_CDS					A1


uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

uint16_t taskLedToggle(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
	return 0;
}

uint16_t actSensorAcquisition(PifSensor* p_owner)
{
	(void)p_owner;

	return analogRead(PIN_CDS);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_CDS, INPUT);

	Serial.begin(115200); //Doesn't matter speed

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
