// Do not remove the include below
#include "exFnd4S.h"

#include "pifFnd.h"
#include "pifLog.h"
#include "pifTask.h"


#define PIN_LED_L				13

#define FND_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              5


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stFnd *s_pstFnd = NULL;

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


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _actFnd1Display(uint8_t ucSegment, uint8_t ucDigit)
{
	for (int j = 0; j < 4; j++) {
		digitalWrite(c_unPinCom[j], j != ucDigit);
	}

	for (int j = 0; j < 8; j++) {
		digitalWrite(c_unPinFnd[j], (ucSegment >> j) & 1);
	}
}

static void _taskFndTest(PIF_stTask *pstTask)
{
	static int nBlink = 0;
	static BOOL swLed = LOW;
	static BOOL swBlink = FALSE;
	static BOOL swFloat = FALSE;

	(void)pstTask;

	if (swFloat) {
		pifFnd_SetSubNumericDigits(s_pstFnd, 0);
		int32_t nValue = rand() % 14000 - 2000;
		if (nValue <= -1000) {
			pifFnd_SetString(s_pstFnd, (char *)"UDER");
		}
		else if (nValue < 10000) {
			pifFnd_SetInterger(s_pstFnd, nValue);
		}
		else {
			pifFnd_SetString(s_pstFnd, (char *)"OVER");
		}

		pifLog_Printf(LT_enInfo, "Blink:%d Float:%d Value:%d", swBlink, swFloat, nValue);
	}
	else {
		pifFnd_SetSubNumericDigits(s_pstFnd, 1);
		double dValue = (rand() % 11000 - 1000) / 10.0;
		pifFnd_SetFloat(s_pstFnd, dValue);

		pifLog_Printf(LT_enInfo, "Blink:%d Float:%d Value:%1f", swBlink, swFloat, dValue);
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

	for (int i = 0; i < 8; i++) {
		pinMode(c_unPinFnd[i], OUTPUT);
	}
	for (int i = 0; i < 4; i++) {
		pinMode(c_unPinCom[i], OUTPUT);
	}

	Serial.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    if (!pifFnd_Init(s_pstTimer, FND_COUNT)) return;
    s_pstFnd = pifFnd_Add(1, 4, _actFnd1Display);
    if (!s_pstFnd) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;	// 100%
    if (!pifTask_AddRatio(5, pifFnd_taskAll, NULL)) return;		// 5%

    if (!pifTask_AddPeriod(1000, _taskFndTest, NULL)) return;	// 1000 * 1ms = 1sec

    pifFnd_Start(s_pstFnd);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
