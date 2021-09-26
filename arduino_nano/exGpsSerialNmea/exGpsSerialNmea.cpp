// Do not remove the include below
#include <MsTimer2.h>
#include <SoftwareSerial.h>

#include "exGpsSerialNmea.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH			2


SoftwareSerial serialGps(8, 9);		// RX, TX


void actLedLState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial.write((char *)pucBuffer, usSize);
}

uint16_t actPushSwitchAcquire(PIF_usId usPifId)
{
	(void)usPifId;

	return !digitalRead(PIN_PUSH_SWITCH);
}

uint16_t actGpsSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return serialGps.write((char *)pucBuffer, usSize);
}

BOOL actGpsReceiveData(PIF_stComm *pstOwner, uint8_t *pucData)
{
	int rxData;

	(void)pstOwner;

	rxData = serialGps.read();
	if (rxData >= 0) {
		*pucData = rxData;
		if (g_bPrintRawData) Serial.write(rxData);
		return TRUE;
	}
	return FALSE;
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
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);
	serialGps.begin(9600);

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
