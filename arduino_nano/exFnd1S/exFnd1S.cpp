// Do not remove the include below
#include <MsTimer2.h>

#include "exFnd1S.h"
#include "appMain.h"


#define PIN_LED_L				13


const uint8_t c_unPinFnd[] = {
		3,		// a
		2, 		// b
		5, 		// c
		6, 		// d
		7, 		// e
		8, 		// f
		9,	 	// g
		4 		// dp
};


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit)
{
	(void)ucDigit;

	for (int j = 0; j < 8; j++) {
		digitalWrite(c_unPinFnd[j], (ucSegment >> j) & 1);
	}
}

uint16_t taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL swLed = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
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

	for (int i = 0; i < 8; i++) {
		pinMode(c_unPinFnd[i], OUTPUT);
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
