// Do not remove the include below
#include "exXmodemSerialRx.h"
#include "appMain.h"


#define PIN_LED_L				13

//#define USE_SERIAL_USB			// Linux or Windows
#define USE_SERIAL_3			// Other Anduino


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

BOOL actXmodemSendData(PIF_stRingBuffer *pstBuffer)
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

void actXmodemReceiveData(PIF_stRingBuffer *pstBuffer)
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

		pifPulse_sigTick(g_pstTimer1ms);
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);
#ifdef USE_SERIAL_USB
	SerialUSB.begin(9600);
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
