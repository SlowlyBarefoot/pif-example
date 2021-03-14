// Do not remove the include below
#include "exProtocolSerialS.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31

//#define USE_SERIAL_USB		// Linux or Windows
#define USE_SERIAL_3			// Other Anduino


static uint8_t s_ucPinSwitch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState)
{
	(void)usPifId;
	(void)ucIndex;

	digitalWrite(PIN_LED_L, swState);
}

uint16_t actPushSwitchAcquire(PIF_usId usPifId)
{
	return !digitalRead(s_ucPinSwitch[usPifId - PIF_ID_SWITCH]);
}

BOOL actSerialSendData(PIF_stRingBuffer *pstBuffer)
{
	uint8_t txData;

    while (pifRingBuffer_GetByte(pstBuffer, &txData)) {
#ifdef USE_SERIAL_USB
    	SerialUSB.print((char)txData);
#endif
#ifdef USE_SERIAL_3
    	Serial3.print((char)txData);
#endif
    }
    return TRUE;
}

void actSerialReceiveData(PIF_stRingBuffer *pstBuffer)
{
	uint16_t size = pifRingBuffer_GetRemainSize(pstBuffer);
	int rxData;

    for (uint16_t i = 0; i < size; i++) {
#ifdef USE_SERIAL_USB
    	rxData = SerialUSB.read();
#endif
#ifdef USE_SERIAL_3
		rxData = Serial3.read();
#endif
		if (rxData >= 0) {
			pifRingBuffer_PutByte(pstBuffer, rxData);
		}
		else break;
    }
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(g_pstTimer1ms);
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

    pifTask_Loop();
}
