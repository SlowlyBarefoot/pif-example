// Do not remove the include below
#include <MsTimer2.h>

#include "exFnd4S.h"

#include "pifFnd.h"
#include "pifLog.h"
#include "pifTask.h"


#define PIN_NANO_LED			13

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              5


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stFnd *s_pstFnd = NULL;

const uint8_t c_unPinFnd[] = {
		6,		// a
		10,		// b
		11,		// c
		3, 		// d
		2, 		// e
		7, 		// f
		12,	 	// g
		4 		// dp
};

const uint8_t c_unPinCom[] = {
		5,		// COM1
		8,		// COM2
		9,		// COM3
		13		// COM4
};


static void _LogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _Fnd1Display(uint8_t ucSegment, uint8_t ucDigit, uint8_t ucColor)
{
	(void)ucColor;

	for (int j = 0; j < 8; j++) {
		digitalWrite(c_unPinFnd[j], (ucSegment >> j) & 1);
	}

	for (int j = 0; j < 4; j++) {
		digitalWrite(c_unPinCom[j], !((ucDigit >> j) & 1));
	}
}

static void _FndTest(void *pvIssuer)
{
	static int nBlink = 0;
	static BOOL swLed = LOW;
	static BOOL swBlink = FALSE;
	static BOOL swFloat = FALSE;

	(void)pvIssuer;

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
	}
	else {
		pifFnd_SetSubNumericDigits(s_pstFnd, 1);
		double dValue = (rand() % 11000 - 1000) / 10.0;
		pifFnd_SetFloat(s_pstFnd, dValue);
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

	digitalWrite(PIN_NANO_LED, swLed);
	swLed ^= 1;

	pifLog_Printf(LT_enInfo, "Blink:%d Float:%d", swBlink, swFloat);
}

void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer);
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_stPulseItem *pstTimer1ms;

	pinMode(PIN_NANO_LED, OUTPUT);

	for (int i = 0; i < 8; i++) {
		pinMode(c_unPinFnd[i], OUTPUT);
	}
	for (int i = 0; i < 4; i++) {
		pinMode(c_unPinCom[i], OUTPUT);
	}

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_LogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    if (!pifFnd_Init(s_pstTimer, 4)) return;
    s_pstFnd = pifFnd_AddSingle(1, 4, 8, _Fnd1Display);
    if (!s_pstFnd) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_LoopAll, NULL)) return;
    if (!pifTask_AddRatio(5, pifFnd_LoopAll, NULL)) return;

    pifFnd_Start(s_pstFnd);

    pstTimer1ms = pifPulse_AddItem(s_pstTimer, PT_enRepeat);
    if (!pstTimer1ms) return;
    pifPulse_AttachEvtFinish(pstTimer1ms, _FndTest, NULL);
    pifPulse_StartItem(pstTimer1ms, 1000);	// 1000 * 1ms = 1sec
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
