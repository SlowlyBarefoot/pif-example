#include "ex_ptr_array.h"
#include "app_port.h"


#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
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
	int line;
	static PifUart s_uart_log;

	Serial.begin(UART_LOG_BAUDRATE);

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) {
		line = __LINE__;
		goto fail;
	}

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) {
		line = __LINE__;
		goto fail;
	}
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) {
		line = __LINE__;
		goto fail;
	}
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 1024)) {								// 1024bytes
		line = __LINE__;
		goto fail;
	}
	return TRUE;

fail:	
	pifLog_Printf(LT_ERROR, "Error: %d Line=%d\n", pif_error, line);
	return FALSE;
}
