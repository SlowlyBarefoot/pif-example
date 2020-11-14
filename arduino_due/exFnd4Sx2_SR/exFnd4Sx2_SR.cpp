// Do not remove the include below
#include "exFnd4Sx2_SR.h"

#include "pifFnd.h"
#include "pifLog.h"
#include "pifTask.h"


#define PIN_LED_L				13

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              5

#define PIN_FND_COM1			0
#define PIN_FND_COM2			1
#define PIN_FND_COM3			2
#define PIN_FND_COM4			3
#define PIN_74HC595_SHIFT		4
#define PIN_74HC595_LATCH		5
#define PIN_74HC595_DATA		6


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stFnd *s_pstFnd = NULL;

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


static void _LogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _Fnd1Display(uint8_t ucSegment, uint8_t ucDigit, uint8_t ucColor)
{
	int block = ucDigit >> 2;

	(void)ucColor;

	for (int j = 0; j < 4; j++) {
		digitalWrite(c_unPin[0][j], j != ucDigit);
		digitalWrite(c_unPin[1][j], j + 4 != ucDigit);
	}

	digitalWrite(c_unPin[block][PIN_74HC595_LATCH], LOW);
	shiftOut(c_unPin[block][PIN_74HC595_DATA], c_unPin[block][PIN_74HC595_SHIFT], MSBFIRST, ucSegment);
	digitalWrite(c_unPin[block][PIN_74HC595_LATCH], HIGH);
}

static void _FndTest(PIF_stTask *pstTask)
{
	static int nBlink = 0;
	static BOOL swLed = LOW;
	static BOOL swBlink = FALSE;
	static BOOL swFloat = FALSE;

	(void)pstTask;

	if (swFloat) {
		pifFnd_SetSubNumericDigits(s_pstFnd, 0);
		int32_t nValue = rand() % 1400000 - 200000;
		if (nValue <= -100000) {
			pifFnd_SetString(s_pstFnd, (char *)" UNDER");
		}
		else if (nValue < 1000000) {
			pifFnd_SetInterger(s_pstFnd, nValue);
		}
		else {
			pifFnd_SetString(s_pstFnd, (char *)"  OVER");
		}

		pifLog_Printf(LT_enInfo, "Blink:%d Float:%d Value:%d", swBlink, swFloat, nValue);
	}
	else {
		pifFnd_SetSubNumericDigits(s_pstFnd, 2);
		double dValue = (rand() % 1100000 - 100000) / 100.0;
		pifFnd_SetFloat(s_pstFnd, dValue);

		pifLog_Printf(LT_enInfo, "Blink:%d Float:%d Value:%2f", swBlink, swFloat, dValue);
	}
	swFloat ^= 1;
	nBlink = (nBlink + 1) % 20;
	if (!nBlink) {
		if (swBlink) {
		    pifFnd_BlinkOff(s_pstFnd);
		}
		else {
		    pifFnd_BlinkOn(s_pstFnd, 200);
		}
		swBlink ^= 1;
	}

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(s_pstTimer);
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

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_LogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    if (!pifFnd_Init(s_pstTimer, 4)) return;
    s_pstFnd = pifFnd_Add(1, 8, 8, _Fnd1Display);
    if (!s_pstFnd) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;
    if (!pifTask_AddRatio(5, pifFnd_taskAll, NULL)) return;

    if (!pifTask_AddPeriod(3000, _FndTest, NULL)) return;	// 1000 * 1ms = 1sec

    pifFnd_Start(s_pstFnd);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
