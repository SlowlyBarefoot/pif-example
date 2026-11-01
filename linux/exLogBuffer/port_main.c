#include "host.h"
#include "app_port.h"


#define UART_LOG_BAUDRATE		115200

#define LOG_BUFFER_SIZE			0x400


static uint8_t s_aucLog[LOG_BUFFER_SIZE];
static PifUart s_uart_log;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return hostConsoleWrite(pucBuffer, usSize);
}

// The log in the buffer stands for the LED here. Drawing it as the other examples do would break into the lines
// printed from the buffer.
static void _evtLedToggle(void *pvIssuer)
{
	static BOOL sw = FALSE;
	static int count = 19;
	static BOOL attached = FALSE;

	(void)pvIssuer;

	sw ^= 1;

	pifLog_Printf(LT_INFO, "LED: %u", sw);

	// The UART attached late gets what the log buffer still holds, and every line after it.
	if (count) count--;
	else if (!attached) {
		pifLog_AttachUart(&s_uart_log, 256);														// 256bytes
		attached = TRUE;
	}
}

// Body of the SysTick handler of a board, called from the tick thread of host.c.
void portTick1ms()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

BOOL portSetup()
{
    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;
	pifTaskManager_SetIdle(hostEvtIdle, 0);

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

    // No UART yet: the lines wait in the log buffer.
    if (!pifLog_InitStatic(LOG_BUFFER_SIZE, s_aucLog)) return FALSE;

	g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_led) return FALSE;
    pifTimer_AttachEvtFinish(g_timer_led, _evtLedToggle, NULL, FALSE);
	return TRUE;
}
