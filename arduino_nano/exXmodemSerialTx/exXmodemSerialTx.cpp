// Do not remove the include below
#include <MsTimer2.h>
#include <SoftwareSerial.h>

#include "exXmodemSerialTx.h"
#include "appMain.h"


#define PIN_LED_L				13


static SoftwareSerial SwSerial(7, 8);


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

BOOL actXmodemSendData(PIF_stRingBuffer *pstBuffer)
{
	uint8_t txData;

    while (pifRingBuffer_GetByte(pstBuffer, &txData)) {
    	SwSerial.write((char)txData);
    }
    return TRUE;
}

void actXmodemReceiveData(PIF_stRingBuffer *pstBuffer)
{
	uint16_t size = pifRingBuffer_GetRemainSize(pstBuffer);
	int rxData;

	for (uint16_t i = 0; i < size; i++) {
		rxData = SwSerial.read();
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

static void sysTickHook()
{
    pif_sigTimer1ms();

	pifPulse_sigTick(g_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed
	SwSerial.begin(115200);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
