#include "appMain.h"
#include "main.h"

#include "core/pif_log.h"


BOOL appInit()
{
	static PifUart s_uart_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return FALSE;

    pifLog_Init();

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return FALSE;

    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            exLogPrint            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
    return TRUE;
}

void appExit()
{
    pifLog_Clear();
    pif_Exit();
}
