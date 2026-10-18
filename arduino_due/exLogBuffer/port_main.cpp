#include "exLogBuffer.h"
#include "app_port.h"


#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200

#define LOG_BUFFER_SIZE			0x200


static uint8_t s_aucLog[LOG_BUFFER_SIZE];
static PifTask* s_task_print_buffer;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint32_t _taskPrintBuffer(PifTask *p_task)
{
	// The UART takes only part of the retained buffer at a time, so come back until it is empty.
	if (!pifLog_PrintInBuffer()) pifTask_Pause(p_task);
	return 0;
}

static void _evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;
	static int count = 19;

	(void)pvIssuer;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;

	pifLog_Printf(LT_INFO, "LED: %u", sw);

	if (count) count--;
	else {
	    pifTask_Resume(s_task_print_buffer);
	    count = 19;
	}
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

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

    if (!pifLog_InitStatic(LOG_BUFFER_SIZE, s_aucLog)) return FALSE;
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;										// 256bytes

    s_task_print_buffer = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskPrintBuffer, NULL, FALSE);	// 1ms
    if (!s_task_print_buffer) return FALSE;

	g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_led) return FALSE;
    pifTimer_AttachEvtFinish(g_timer_led, _evtLedToggle, NULL, FALSE);
	return TRUE;
}
