// Do not remove the include below
#include <MsTimer2.h>

#include "exFnd4S.h"
#include "appMain.h"


#define PIN_LED_L				13


const uint8_t c_unPinFnd[] = {
		6,		// a
		10,		// b
		11,		// c
		3, 		// d
		2, 		// e
		7, 		// f
		12,	 	// g
		4 		// dp
};

const uint8_t c_unPinCom[] = {
		5,		// COM1
		8,		// COM2
		9,		// COM3
		13		// COM4
};


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
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

void taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL swLed = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
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

	for (int i = 0; i < 8; i++) {
		pinMode(c_unPinFnd[i], OUTPUT);
	}
	for (int i = 0; i < 4; i++) {
		pinMode(c_unPinCom[i], OUTPUT);
	}

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
