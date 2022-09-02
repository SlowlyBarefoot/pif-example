// Do not remove the include below
#include "ex_gps_serial_ublox.h"
#include "app_main.h"

#include "pif_log.h"


#define PIN_LED_L				13


void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial.write((char *)pucBuffer, usSize);
}

BOOL actLogReceiveData(PifComm *pstOwner, uint8_t *pucData)
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

void actGpsSetBaudrate(uint32_t baudrate)
{
	Serial1.begin(baudrate);
}

uint16_t actGpsSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial1.write((char *)pucBuffer, usSize);
}

BOOL actGpsReceiveData(PifComm *pstOwner, uint8_t *pucData)
{
	int rxData;

	(void)pstOwner;

	rxData = Serial1.read();
	if (rxData >= 0) {
		*pucData = rxData;
		if (g_print_data == 2) Serial.write(rxData);
		return TRUE;
	}
	return FALSE;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
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
	pifTaskManager_Loop();
}
