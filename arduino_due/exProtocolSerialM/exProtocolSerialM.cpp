// Do not remove the include below
#include "exProtocolSerialM.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31


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
    	SerialUSB.print((char)txData);
    }
    return TRUE;
}

void actSerialReceiveData(PIF_stRingBuffer *pstBuffer)
{
	uint16_t size = pifRingBuffer_GetRemainSize(pstBuffer);
	int rxData;

	for (uint16_t i = 0; i < size; i++) {
		rxData = SerialUSB.read();
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
	SerialUSB.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
