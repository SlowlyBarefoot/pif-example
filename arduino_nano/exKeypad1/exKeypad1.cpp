// Do not remove the include below
#include <MsTimer2.h>

#include "exKeypad1.h"

#include "pifKeypad.h"
#include "pifLog.h"
#include "pifPulse.h"


#define PIN_LED_L				13

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              2


static PIF_stPulse *s_pstTimer1ms = NULL;

const uint8_t ROWS = 4;    // 행(rows) 개수
const uint8_t COLS = 4;    // 열(columns) 개수
const char keys[ROWS * COLS] = {
	'1', '2', '3', 'A',
	'4', '5', '6', 'B',
	'7', '8', '9', 'C',
	'*', '0', '#', 'D'
};

uint8_t rowPins[ROWS] = { 6, 7, 8, 9 };   // R1, R2, R3, R4 단자가 연결된 아두이노 핀 번호
uint8_t colPins[COLS] = { 5, 4, 3, 2 };   // C1, C2, C3, C4 단자가 연결된 아두이노 핀 번호


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _actLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

static void _actKeypadAcquire(uint16_t *pusState)
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

static void _evtKeypadPressed(char cChar)
{
	pifLog_Printf(LT_enInfo, "Keypad:Pressed %c", cChar);
}

static void _evtKeypadReleased(char cChar, uint32_t unOnTime)
{
	pifLog_Printf(LT_enInfo, "Keypad:Released %c T:%lu", cChar, unOnTime);
}

static void _evtKeypadLongReleased(char cChar, uint32_t unOnTime)
{
	pifLog_Printf(LT_enInfo, "Keypad:LongReleased %c T:%lu", cChar, unOnTime);
}

static void _evtKeypadDoublePressed(char cChar)
{
	pifLog_Printf(LT_enInfo, "Keypad:DoublePressed %c", cChar);
}

static void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	int r;
	PIF_stKeypad *pstKeypad;

	pinMode(PIN_LED_L, OUTPUT);
	for (r = 0; r < ROWS; r++) {
		pinMode(rowPins[r], INPUT_PULLUP);
	}

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

	pif_Init();

	pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;

    pstKeypad = pifKeypad_Init(PIF_ID_AUTO, ROWS, COLS, keys);
    if (!pstKeypad) return;
    pifKeypad_AttachAction(_actKeypadAcquire);
    pstKeypad->evtPressed = _evtKeypadPressed;
    pstKeypad->evtReleased = _evtKeypadReleased;
    pstKeypad->evtLongReleased = _evtKeypadLongReleased;
    pstKeypad->evtDoublePressed = _evtKeypadDoublePressed;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriodMs(10, pifKeypad_taskAll, NULL)) return;	// 10ms

    if (!pifTask_AddPeriodMs(500, _actLedToggle, NULL)) return;		// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
