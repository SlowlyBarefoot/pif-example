// Do not remove the include below
#include <MsTimer2.h>

#include "exFnd1S.h"

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
		3,		// a
		2, 		// b
		5, 		// c
		6, 		// d
		7, 		// e
		8, 		// f
		9,	 	// g
		4 		// dp
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

static void _FndTest(void *pvIssuer)
{
	static int i = 0;
	static BOOL swLed = LOW;
	static BOOL swBlink = FALSE;
	char buf[2];

	(void)pvIssuer;

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

	digitalWrite(PIN_NANO_LED, swLed);
	swLed ^= 1;

	pifLog_Printf(LT_enInfo, "%d", i);
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
    s_pstFnd = pifFnd_AddSingle(1, 1, 8, _Fnd1Display);
    if (!s_pstFnd) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) return;
    if (!pifTask_Add(5, pifFnd_LoopAll, NULL)) return;

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
