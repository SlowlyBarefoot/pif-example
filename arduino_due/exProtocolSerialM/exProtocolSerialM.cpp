// Do not remove the include below
#include "exProtocolSerialM.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31

#define USE_SERIAL_USB			// Linux or Windows
//#define USE_SERIAL_3			// Other Anduino


static uint8_t s_ucPinSwitch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


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
	return !digitalRead(s_ucPinSwitch[usPifId - PIF_ID_SWITCH]);
}

uint16_t actSerialSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

#ifdef USE_SERIAL_USB
    return SerialUSB.write((char *)pucBuffer, usSize);
#endif
#ifdef USE_SERIAL_3
    return Serial3.write((char *)pucBuffer, usSize);
#endif
}

BOOL actSerialReceiveData(PifComm *pstComm, uint8_t *pucData)
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
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

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
