// Do not remove the include below
#include "exDotMatrixS.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_1	23
#define PIN_2	25
#define PIN_3	27
#define PIN_4	29
#define PIN_5	31
#define PIN_6	33
#define PIN_7	35
#define PIN_8	37
#define PIN_9	39
#define PIN_10	41
#define PIN_11	43
#define PIN_12	45
#define PIN_13	47
#define PIN_14	49
#define PIN_15	51
#define PIN_16	53


const uint8_t c_ucPinDotMatrixCol[] = { PIN_13, PIN_3, PIN_4, PIN_10, PIN_6, PIN_11, PIN_15, PIN_16 };
const uint8_t c_ucPinDotMatrixRow[] = { PIN_9, PIN_14, PIN_8, PIN_12, PIN_1, PIN_7, PIN_2, PIN_5 };


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actDotMatrixDisplay(uint8_t ucRow, uint8_t *pucData)
{
	static int row = -1;

	if (row >= 0) digitalWrite(c_ucPinDotMatrixRow[row], LOW);

	digitalWrite(c_ucPinDotMatrixRow[ucRow], HIGH);
	for (int col = 0; col < 8; col++) {
		digitalWrite(c_ucPinDotMatrixCol[col], !((*pucData >> col) & 1));
	}
	row = ucRow;
}

void taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL swLed = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
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

	for (int i = 0; i < 8; i++) {
		pinMode(c_ucPinDotMatrixCol[i], OUTPUT);
		pinMode(c_ucPinDotMatrixRow[i], OUTPUT);
	}

	Serial.begin(115200); //Doesn't matter speed

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
