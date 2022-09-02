// Do not remove the include below
#include "exFnd4Sx2_SR.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_FND_COM1			0
#define PIN_FND_COM2			1
#define PIN_FND_COM3			2
#define PIN_FND_COM4			3
#define PIN_74HC595_SHIFT		4
#define PIN_74HC595_LATCH		5
#define PIN_74HC595_DATA		6


const uint8_t c_unPin[2][7] = {
		{			// FND 1
			41,		// COM1
			43,		// COM2
			45,		// COM3
			47,		// COM4
			49,		// 74HC595 Shift
			51,		// 74HC595 Latch
			53		// 74HC595 Data
		},
		{			// FND 2
			27,		// COM1
			29,		// COM2
			31,		// COM3
			33,		// COM4
			35,		// 74HC595 Shift
			37,		// 74HC595 Latch
			39		// 74HC595 Data
		}
};


uint16_t actLogSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit)
{
	int block = ucDigit >> 2;

	for (int j = 0; j < 4; j++) {
		digitalWrite(c_unPin[0][j], j != ucDigit);
		digitalWrite(c_unPin[1][j], j + 4 != ucDigit);
	}

	digitalWrite(c_unPin[block][PIN_74HC595_LATCH], LOW);
	shiftOut(c_unPin[block][PIN_74HC595_DATA], c_unPin[block][PIN_74HC595_SHIFT], MSBFIRST, ucSegment);
	digitalWrite(c_unPin[block][PIN_74HC595_LATCH], HIGH);
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

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 7; j++) {
			pinMode(c_unPin[i][j], OUTPUT);
		}
	}

	Serial.begin(115200); //Doesn't matter speed

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
