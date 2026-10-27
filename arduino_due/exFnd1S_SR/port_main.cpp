#include "exFnd1S_SR.h"
#include "app_port.h"


#define PIN_LED_L				13

#define PIN_74HC595_DATA		26
#define PIN_74HC595_LATCH		24
#define PIN_74HC595_SHIFT		22

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actFndDisplay(PifId id, uint8_t ucSegment, uint8_t ucDigit)
{
	(void)id;
	(void)ucDigit;

	digitalWrite(PIN_74HC595_LATCH, FALSE);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, ucSegment);
	digitalWrite(PIN_74HC595_LATCH, TRUE);
}

static uint32_t taskLedToggle(PifTask *pstTask)
{
	static BOOL swLed = FALSE;

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

	pinMode(PIN_74HC595_DATA, OUTPUT);
	pinMode(PIN_74HC595_LATCH, OUTPUT);
	pinMode(PIN_74HC595_SHIFT, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;												// 256bytes

    if (!pifFnd_Init(&g_fnd, PIF_ID_AUTO, 1, actFndDisplay, NULL)) return FALSE;
    g_fnd._p_task->name = "FND";

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return FALSE;	// 500ms
	return TRUE;
}
