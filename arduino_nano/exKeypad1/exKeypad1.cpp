// Do not remove the include below
#include <MsTimer2.h>

#include "exKeypad1.h"
#include "appMain.h"


#define PIN_LED_L				13


static uint8_t rowPins[ROWS] = { 6, 7, 8, 9 };   // R1, R2, R3, R4 단자가 연결된 아두이노 핀 번호
static uint8_t colPins[COLS] = { 5, 4, 3, 2 };   // C1, C2, C3, C4 단자가 연결된 아두이노 핀 번호


uint16_t actLogSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

uint16_t taskLedToggle(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
	return 0;
}

void actKeypadAcquire(uint16_t *pusState)
{
	int row, col, i, block, cell;

	for (i = col = 0; col < COLS; col++) {
		pinMode(colPins[col], OUTPUT);
		digitalWrite(colPins[col], LOW);
		for (row = 0; row < ROWS; row++, i++) {
			block = i / 16;
			cell = i & 15;
			if (!digitalRead(rowPins[row])) {
				pusState[block] |= 1UL << cell;
			}
			else {
				pusState[block] &= ~(1UL << cell);
			}
		}
		digitalWrite(colPins[col], HIGH);
		pinMode(colPins[col],INPUT);
	}
}

static void sysTickHook()
{
	pif_sigTimer1ms();
}

//The setup function is called once at startup of the sketch
void setup()
{
	int r;

	pinMode(PIN_LED_L, OUTPUT);
	for (r = 0; r < ROWS; r++) {
		pinMode(rowPins[r], INPUT_PULLUP);
	}

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
