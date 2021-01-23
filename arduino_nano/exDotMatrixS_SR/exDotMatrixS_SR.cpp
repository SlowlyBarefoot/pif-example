// Do not remove the include below
#include <MsTimer2.h>

#include "exDotMatrixS_SR.h"

#include "pifDotMatrix.h"
#include "pifLog.h"
#include "pifTask.h"


#define PIN_LED_L				13

#define PIN_74HC595_DATA		7
#define PIN_74HC595_LATCH		6
#define PIN_74HC595_SHIFT		5

#define DOT_MATRIX_COUNT        1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              3


const uint8_t font8x8_basic[10][8] = {
    { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
    { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
    { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
    { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
    { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
    { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
    { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
    { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
    { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
    { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
};

static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stDotMatrix *s_pstDotMatrix = NULL;


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _actDotMatrixDisplay(uint8_t ucRow, uint8_t *pucData)
{
	digitalWrite(PIN_74HC595_LATCH, LOW);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, 1 << ucRow);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, ~(*pucData));
	digitalWrite(PIN_74HC595_LATCH, HIGH);
}

static void _taskDotMatrixTest(PIF_stTask *pstTask)
{
	static BOOL swLed = LOW;
	static int nBlink = 0;
	static int index = 0;

	(void)pstTask;

	pifDotMatrix_SelectPattern(s_pstDotMatrix, index);
	index++;
	if (index >= 10) index = 0;

	nBlink++;
	switch (nBlink) {
	case 10:
	    pifDotMatrix_BlinkOn(s_pstDotMatrix, 200);
	    break;

	case 20:
	    pifDotMatrix_BlinkOff(s_pstDotMatrix);
	    nBlink = 0;
	    break;
	}

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
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

	pinMode(PIN_74HC595_DATA, OUTPUT);
	pinMode(PIN_74HC595_LATCH, OUTPUT);
	pinMode(PIN_74HC595_SHIFT, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;

    if (!pifDotMatrix_Init(s_pstTimer1ms, DOT_MATRIX_COUNT)) return;

    s_pstDotMatrix = pifDotMatrix_Add(PIF_ID_AUTO, 8, 8, _actDotMatrixDisplay);
    if (!s_pstDotMatrix) return;
    if (!pifDotMatrix_SetPatternSize(s_pstDotMatrix, 10)) return;
    for (int i = 0; i < 10; i++) {
    	if (!pifDotMatrix_AddPattern(s_pstDotMatrix, 8, 8, (uint8_t *)font8x8_basic[i])) return;
    }

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
	if (!pifTask_AddRatio(5, pifDotMatrix_taskAll, NULL)) return;			// 5%

	if (!pifTask_AddPeriodMs(1000, _taskDotMatrixTest, NULL)) return;		// 1000ms

	pifDotMatrix_Start(s_pstDotMatrix);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
