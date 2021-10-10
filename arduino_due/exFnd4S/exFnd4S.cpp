// Do not remove the include below
#include "exFnd4S.h"
#include "appMain.h"


#define PIN_LED_L				13


const uint8_t c_unPinFnd[] = {
		43,		// a
		35, 	// b
		33, 	// c
		49, 	// d
		51, 	// e
		41, 	// f
		31, 	// g
		47		// dp
};

const uint8_t c_unPinCom[] = {
		45,		// COM1
		39,		// COM2
		37,		// COM3
		29		// COM4
};


uint16_t actLogSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit)
{
	for (int j = 0; j < 4; j++) {
		digitalWrite(c_unPinCom[j], j != ucDigit);
	}

	for (int j = 0; j < 8; j++) {
		digitalWrite(c_unPinFnd[j], (ucSegment >> j) & 1);
	}
}

uint16_t taskLedToggle(PifTask *pstTask)
{
	static BOOL swLed = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
	return 0;
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

	for (int i = 0; i < 8; i++) {
		pinMode(c_unPinFnd[i], OUTPUT);
	}
	for (int i = 0; i < 4; i++) {
		pinMode(c_unPinCom[i], OUTPUT);
	}

	Serial.begin(115200); //Doesn't matter speed

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
