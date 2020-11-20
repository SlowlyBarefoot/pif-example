// Do not remove the include below
#include <MsTimer2.h>

#include "exFnd4Sx2_SR2.h"

#include "pifFnd.h"
#include "pifLog.h"
#include "pifTask.h"


#define PIN_LED_L				13

#define PIN_74HC595_DATA		5
#define PIN_74HC595_LATCH		7
#define PIN_74HC595_SHIFT		8

#define FND_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              5


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stFnd *s_pstFnd = NULL;


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _actFnd1Display(uint8_t ucSegment, uint8_t ucDigit)
{
	digitalWrite(PIN_74HC595_LATCH, LOW);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, ~(1 << ucDigit));
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, ucSegment);
	digitalWrite(PIN_74HC595_LATCH, HIGH);
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
		int32_t nValue = rand() % 140000 - 20000;
		if (nValue <= -10000) {
			pifFnd_SetString(s_pstFnd, (char *)" UNDER");
		}
		else if (nValue < 100000) {
			pifFnd_SetInterger(s_pstFnd, nValue);
		}
		else {
			pifFnd_SetString(s_pstFnd, (char *)"  OVER");
		}

		pifLog_Printf(LT_enInfo, "Blink:%d Float:%d Value:%d", swBlink, swFloat, nValue);
	}
	else {
		pifFnd_SetSubNumericDigits(s_pstFnd, 2);
		double dValue = (rand() % 110000 - 10000) / 100.0;
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

static void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	pinMode(PIN_74HC595_DATA, OUTPUT);
	pinMode(PIN_74HC595_LATCH, OUTPUT);
	pinMode(PIN_74HC595_SHIFT, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    if (!pifFnd_Init(s_pstTimer, FND_COUNT)) return;
    s_pstFnd = pifFnd_Add(1, 8, _actFnd1Display);
    if (!s_pstFnd) return;
    pifFnd_SetControlPeriod(s_pstFnd, 16);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;	// 100%
    if (!pifTask_AddRatio(5, pifFnd_taskAll, NULL)) return;		// 5%

    if (!pifTask_AddPeriod(3000, _taskFndTest, NULL)) return;	// 3000 * 1ms = 3sec

    pifFnd_Start(s_pstFnd);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
