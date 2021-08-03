// Do not remove the include below
#include <MsTimer2.h>

#include "exTerminal.h"
#include "appMain.h"


#define PIN_LED_L				13


uint16_t actSerialSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

BOOL actSerialReceiveData(PIF_stComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = Serial.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

uint16_t taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
	return 0;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
