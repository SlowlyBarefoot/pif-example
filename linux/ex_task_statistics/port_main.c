#include "host.h"
#include "app_port.h"


#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return hostConsoleWrite(pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	(void)p_uart;

	return hostConsoleRead(p_data, size);
}

// There is no pin to put a scope on. The RT line and the VCD file show the jitter instead.
void actRealtimeSignal(BOOL state)
{
	(void)state;
}

// Body of the SysTick handler of a board, called from the tick thread of host.c.
void portTick1ms()
{
	pif_sigTimer1ms();
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;
	pifTaskManager_SetIdle(hostEvtIdle, 0);

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 20000, NULL)) return FALSE;		// 20ms
	s_uart_log.act_receive_data = actLogReceiveData;
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	// The table of the task command goes out at once, more than the 256 bytes the other examples keep.
	if (!pifLog_AttachUart(&s_uart_log, 1024)) return FALSE;										// 1024bytes
	return TRUE;
}
