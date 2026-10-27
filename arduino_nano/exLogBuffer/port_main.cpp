#include "exLogBuffer.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200

#define LOG_BUFFER_SIZE			0x200


static uint8_t s_aucLog[LOG_BUFFER_SIZE];


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void _evtLedToggle(void *pvIssuer)
{
	static BOOL sw = FALSE;
	static int count = 19;

	(void)pvIssuer;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;

	pifLog_Printf(LT_INFO, "LED: %u", sw);

	if (count) count--;
	else {
	    pifLog_PrintInBuffer();
	    count = 19;
	}
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

    if (!pifLog_InitStatic(LOG_BUFFER_SIZE, s_aucLog)) return FALSE;
	if (!pifLog_AttachUart(&s_uart_log, 80)) return FALSE;										// 80bytes

	g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_led) return FALSE;
    pifTimer_AttachEvtFinish(g_timer_led, _evtLedToggle, NULL, FALSE);
	return TRUE;
}
