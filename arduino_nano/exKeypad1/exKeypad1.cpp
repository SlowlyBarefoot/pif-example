// Do not remove the include below
#include <MsTimer2.h>

#include "exKeypad1.h"
#include "appMain.h"


#define PIN_LED_L				13

#define TASK_SIZE				3


static uint8_t rowPins[ROWS] = { 6, 7, 8, 9 };   // R1, R2, R3, R4 단자가 연결된 아두이노 핀 번호
static uint8_t colPins[COLS] = { 5, 4, 3, 2 };   // C1, C2, C3, C4 단자가 연결된 아두이노 핀 번호


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t taskLedToggle(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
	return 0;
}

static void actKeypadAcquire(uint16_t *pusState)
{
	int row, col, i, block, cell;

	for (i = col = 0; col < COLS; col++) {
		pinMode(colPins[col], OUTPUT);
		digitalWrite(colPins[col], LOW);
		for (row = 0; row < ROWS; row++, i++) {
			block = i / 16;
			cell = i & 15;
			if (!digitalRead(rowPins[row])) {
				pusState[block] |= 1UL << cell;
			}
			else {
				pusState[block] &= ~(1UL << cell);
			}
		}
		digitalWrite(colPins[col], HIGH);
		pinMode(colPins[col],INPUT);
	}
}

static void sysTickHook()
{
	pif_sigTimer1ms();
}

//The setup function is called once at startup of the sketch
void setup()
{
	int r;
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	for (r = 0; r < ROWS; r++) {
		pinMode(rowPins[r], INPUT_PULLUP);
	}

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifKeypad_Init(&g_keypad, PIF_ID_AUTO, actKeypadAcquire)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
