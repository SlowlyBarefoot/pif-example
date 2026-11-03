#include "host.h"
#include "app_port.h"


#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return hostConsoleWrite(pucBuffer, usSize);
}

// Body of the SysTick handler of a board, called from the tick thread of host.c.
void portTick1ms()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

// One pulse on the STEP pin of a driver such as the A4988, after DIR is set. Nothing is wired here: app_main.c
// counts the pulses.
void portStepPulse(int8_t dir)
{
	(void)dir;
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;
	pifTaskManager_SetIdle(hostEvtIdle, 0);

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;											// 256bytes
	return TRUE;
}
