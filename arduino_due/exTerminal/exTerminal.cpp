// Do not remove the include below
#include "exTerminal.h"
#include "appMain.h"


#define PIN_LED_L				13


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

uint16_t actSerialSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return SerialUSB.write((char *)pucBuffer, usSize);
}

BOOL actSerialReceiveData(PIF_stComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = SerialUSB.read();
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

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);
	SerialUSB.begin(115200); //Doesn't matter speed

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
