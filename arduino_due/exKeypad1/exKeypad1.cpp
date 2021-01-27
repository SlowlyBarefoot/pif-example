// Do not remove the include below
#include "exKeypad1.h"
#include "appMain.h"


#define PIN_LED_L				13


static uint8_t rowPins[ROWS] = { 40, 42, 44, 46 };   // R1, R2, R3, R4 단자가 연결된 아두이노 핀 번호
static uint8_t colPins[COLS] = { 38, 36, 34, 32 };   // C1, C2, C3, C4 단자가 연결된 아두이노 핀 번호


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

void actKeypadAcquire(uint16_t *pusState)
{
	int r, c;

	for (c = 0; c < COLS; c++) {
		pinMode(colPins[c], OUTPUT);
		digitalWrite(colPins[c], LOW);
		for (r = 0; r < ROWS; r++) {
			if (!digitalRead(rowPins[r])) {
				pusState[r] |= 1UL << c;
			}
			else {
				pusState[r] &= ~(1UL << c);
			}
		}
		digitalWrite(colPins[c], HIGH);
		pinMode(colPins[c],INPUT);
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
	int r;

	pinMode(PIN_LED_L, OUTPUT);
	for (r = 0; r < ROWS; r++) {
		pinMode(rowPins[r], INPUT_PULLUP);
	}

	Serial.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
