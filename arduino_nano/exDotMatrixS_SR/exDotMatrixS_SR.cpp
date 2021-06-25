// Do not remove the include below
#include <MsTimer2.h>

#include "exDotMatrixS_SR.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_74HC595_DATA		7
#define PIN_74HC595_LATCH		6
#define PIN_74HC595_SHIFT		5


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actDotMatrixDisplay(uint8_t ucRow, uint8_t *pucData)
{
	digitalWrite(PIN_74HC595_LATCH, LOW);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, 1 << ucRow);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, ~(*pucData));
	digitalWrite(PIN_74HC595_LATCH, HIGH);
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

	pinMode(PIN_74HC595_DATA, OUTPUT);
	pinMode(PIN_74HC595_LATCH, OUTPUT);
	pinMode(PIN_74HC595_SHIFT, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed

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
