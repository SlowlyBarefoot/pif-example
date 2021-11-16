// Do not remove the include below
#include "exFnd1S_SR.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_74HC595_DATA		26
#define PIN_74HC595_LATCH		24
#define PIN_74HC595_SHIFT		22


uint16_t actLogSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit)
{
	(void)ucDigit;

	digitalWrite(PIN_74HC595_LATCH, LOW);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, ucSegment);
	digitalWrite(PIN_74HC595_LATCH, HIGH);
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
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	pinMode(PIN_74HC595_DATA, OUTPUT);
	pinMode(PIN_74HC595_LATCH, OUTPUT);
	pinMode(PIN_74HC595_SHIFT, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
