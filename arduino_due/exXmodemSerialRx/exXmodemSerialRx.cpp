// Do not remove the include below
#include "exXmodemSerialRx.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH			29

#define USE_SERIAL_USB			// Linux or Windows
//#define USE_SERIAL_3			// Other Anduino


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

uint16_t actPushSwitchAcquire(PifId usPifId)
{
	(void)usPifId;

	return !digitalRead(PIN_PUSH_SWITCH);
}

uint16_t actXmodemSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

#ifdef USE_SERIAL_USB
	return SerialUSB.write((char *)pucBuffer, usSize);
#endif
#ifdef USE_SERIAL_3
   	return Serial3.write((char *)pucBuffer, usSize);
#endif
}

BOOL actXmodemReceiveData(PifComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

#ifdef USE_SERIAL_USB
	rxData = SerialUSB.read();
#endif
#ifdef USE_SERIAL_3
	rxData = Serial3.read();
#endif
	if (rxData >= 0) {
		*pucData = rxData;
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
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);

	Serial.begin(115200);
#ifdef USE_SERIAL_USB
	SerialUSB.begin(115200);
#endif
#ifdef USE_SERIAL_3
	Serial3.begin(115200);
#endif

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
