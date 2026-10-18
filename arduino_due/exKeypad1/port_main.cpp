#include "exKeypad1.h"
#include "app_port.h"


#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200


static uint8_t rowPins[ROWS] = { 40, 42, 44, 46 };   // R1, R2, R3, R4 단자가 연결된 아두이노 핀 번호
static uint8_t colPins[COLS] = { 38, 36, 34, 32 };   // C1, C2, C3, C4 단자가 연결된 아두이노 핀 번호


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint32_t taskLedToggle(PifTask *pstTask)
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

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		return 0;
	}
}

BOOL portSetup()
{
	int r;
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	for (r = 0; r < ROWS; r++) {
		pinMode(rowPins[r], INPUT_PULLUP);
	}

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;												// 256bytes

    if (!pifKeypad_Init(&g_keypad, PIF_ID_AUTO, actKeypadAcquire)) return FALSE;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return FALSE;	// 500ms
	return TRUE;
}
