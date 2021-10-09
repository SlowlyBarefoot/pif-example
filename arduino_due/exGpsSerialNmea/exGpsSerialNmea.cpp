// Do not remove the include below
#include "exGpsSerialNmea.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13


void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial.write((char *)pucBuffer, usSize);
}

BOOL actLogReceiveData(PIF_stComm *pstOwner, uint8_t *pucData)
{
	int rxData;

	(void)pstOwner;

	rxData = Serial.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

uint16_t actGpsSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial1.write((char *)pucBuffer, usSize);
}

BOOL actGpsReceiveData(PIF_stComm *pstOwner, uint8_t *pucData)
{
	int rxData;

	(void)pstOwner;

	rxData = Serial1.read();
	if (rxData >= 0) {
		*pucData = rxData;
		if (g_bPrintRawData) Serial.write(rxData);
		return TRUE;
	}
	return FALSE;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifPulse_sigTick(g_pstTimer1ms);
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);
	Serial1.begin(9600);

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
