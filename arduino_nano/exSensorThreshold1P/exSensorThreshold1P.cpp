// Do not remove the include below
#include <MsTimer2.h>

#include "exSensorThreshold1P.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_CDS					A1


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

void taskSensorAcquisition(PIF_stTask *pstTask)
{
	(void)pstTask;

	pifSensorDigital_sigData(g_pstSensor, analogRead(PIN_CDS));
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
	pinMode(PIN_CDS, INPUT);

	Serial.begin(115200); //Doesn't matter speed

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
