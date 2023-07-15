#include "appMain.h"
#include "main.h"

#include "core/pif_log.h"


#define LOG_BUFFER_SIZE			0x200


BOOL appInit()
{
	static PifUart s_uart_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return FALSE;

    if (!pifLog_InitHeap(LOG_BUFFER_SIZE)) return FALSE;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return FALSE;

    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms

	pifLog_Printf(LT_INFO, "Start");

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());

    pifLog_Disable();
    return TRUE;
}

void appExit()
{
	pifLog_Clear();
    pif_Exit();
}
