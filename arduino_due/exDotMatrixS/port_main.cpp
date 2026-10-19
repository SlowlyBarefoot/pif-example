#include "exDotMatrixS.h"
#include "app_port.h"


#define PIN_LED_L				13

#define PIN_1					23
#define PIN_2					25
#define PIN_3					27
#define PIN_4					29
#define PIN_5					31
#define PIN_6					33
#define PIN_7					35
#define PIN_8					37
#define PIN_9					39
#define PIN_10					41
#define PIN_11					43
#define PIN_12					45
#define PIN_13					47
#define PIN_14					49
#define PIN_15					51
#define PIN_16					53

#define UART_LOG_BAUDRATE		115200


const uint8_t c_ucPinDotMatrixCol[] = { PIN_13, PIN_3, PIN_4, PIN_10, PIN_6, PIN_11, PIN_15, PIN_16 };
const uint8_t c_ucPinDotMatrixRow[] = { PIN_9, PIN_14, PIN_8, PIN_12, PIN_1, PIN_7, PIN_2, PIN_5 };


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actDotMatrixDisplay(uint8_t ucRow, uint8_t *pucData)
{
	static int row = -1;

	if (row >= 0) digitalWrite(c_ucPinDotMatrixRow[row], LOW);

	digitalWrite(c_ucPinDotMatrixRow[ucRow], HIGH);
	for (int col = 0; col < 8; col++) {
		digitalWrite(c_ucPinDotMatrixCol[col], !((*pucData >> col) & 1));
	}
	row = ucRow;
}

static uint32_t taskLedToggle(PifTask *pstTask)
{
	static BOOL swLed = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
	return 0;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	for (int i = 0; i < 8; i++) {
		pinMode(c_ucPinDotMatrixCol[i], OUTPUT);
		pinMode(c_ucPinDotMatrixRow[i], OUTPUT);
	}

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;												// 256bytes

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) goto fail;	// 500ms

    if (!pifDotMatrix_Init(&g_dot_matrix, PIF_ID_AUTO, 8, 8, actDotMatrixDisplay)) goto fail;		// 8 x 8 size
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
