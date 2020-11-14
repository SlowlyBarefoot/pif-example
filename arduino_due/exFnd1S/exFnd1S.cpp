// Do not remove the include below
#include "exFnd1S.h"

#include "pifFnd.h"
#include "pifLog.h"
#include "pifTask.h"


#define PIN_LED_L				13

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              5


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stFnd *s_pstFnd = NULL;

const uint8_t c_unPinFnd[] = {
		43,		// a
		45, 	// b
		39, 	// c
		37, 	// d
		35, 	// e
		33, 	// f
		31, 	// g
		41 		// dp
};


static void _LogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _Fnd1Display(uint8_t ucSegment, uint8_t ucDigit, uint8_t ucColor)
{
	(void)ucDigit;
	(void)ucColor;

	for (int j = 0; j < 8; j++) {
		digitalWrite(c_unPinFnd[j], (ucSegment >> j) & 1);
	}
}

static void _FndTest(PIF_stTask *pstTask)
{
	static int i = 0;
	static BOOL swLed = LOW;
	static BOOL swBlink = FALSE;
	char buf[2];

	(void)pstTask;

	if (i < 10) pifFnd_SetInterger(s_pstFnd, i);
	else {
		buf[0] = 'A' + i - 10;
		buf[1] = 0;
		pifFnd_SetString(s_pstFnd, buf);
	}
	i = (i + 1) % 36;
	if (!i) {
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

	pifLog_Printf(LT_enInfo, "%d", i);
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

	Serial.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_LogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    if (!pifFnd_Init(s_pstTimer, 4)) return;
    s_pstFnd = pifFnd_Add(1, 1, 8, _Fnd1Display);
    if (!s_pstFnd) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;
    if (!pifTask_AddRatio(5, pifFnd_taskAll, NULL)) return;

    if (!pifTask_AddPeriod(1000, _FndTest, NULL)) return;	// 1000 * 1ms = 1sec

    pifFnd_Start(s_pstFnd);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
