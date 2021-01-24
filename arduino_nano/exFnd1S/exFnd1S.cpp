// Do not remove the include below
#include <MsTimer2.h>

#include "exFnd1S.h"

#include "pifFnd.h"
#include "pifLog.h"
#include "pifTask.h"


#define PIN_LED_L				13

#define FND_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              5


static PIF_stPulse *s_pstTimer1ms = NULL;
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

const uint8_t c_ucUserChar[] = {
		0x77, /*  A  */	0x7C, /*  b  */ 0x39, /*  C  */ 0x5E, /*  d  */ 	// 0
		0x79, /*  E  */ 0x71, /*  F  */ 0x3D, /*  G  */ 0x76, /*  H  */ 	// 4
		0x30, /*  I  */ 0x1E, /*  J  */ 0x7A, /*  K  */ 0x38, /*  L  */ 	// 8
		0x55, /*  m  */ 0x54, /*  n  */ 0x5C, /*  o  */	0x73, /*  P  */ 	// 12
		0x67, /*  q  */ 0x50, /*  r  */ 0x6D, /*  S  */	0x78, /*  t  */ 	// 16
		0x3E, /*  U  */ 0x7E, /*  V  */ 0x6A, /*  W  */ 0x36, /*  X  */ 	// 20
		0x6E, /*  y  */ 0x49, /*  Z  */										// 24
};


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _actFnd1Display(uint8_t ucSegment, uint8_t ucDigit)
{
	(void)ucDigit;

	for (int j = 0; j < 8; j++) {
		digitalWrite(c_unPinFnd[j], (ucSegment >> j) & 1);
	}
}

static void _taskFndTest(PIF_stTask *pstTask)
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

static void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	for (int i = 0; i < 8; i++) {
		pinMode(c_unPinFnd[i], OUTPUT);
	}

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;

    if (!pifFnd_Init(s_pstTimer1ms, FND_COUNT)) return;
    pifFnd_SetUserChar(c_ucUserChar, 26);
    s_pstFnd = pifFnd_Add(PIF_ID_AUTO, 1, _actFnd1Display);
    if (!s_pstFnd) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;	// 100%
    if (!pifTask_AddRatio(5, pifFnd_taskAll, NULL)) return;		// 5%

    if (!pifTask_AddPeriodMs(1000, _taskFndTest, NULL)) return;	// 1000ms

    pifFnd_Start(s_pstFnd);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
