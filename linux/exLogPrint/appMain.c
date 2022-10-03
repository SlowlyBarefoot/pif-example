#include "appMain.h"
#include "main.h"

#include "core/pif_log.h"


BOOL appInit()
{
	static PifComm s_comm_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return FALSE;

    pifLog_Init();

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return FALSE;
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return FALSE;

    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
    return TRUE;
}

void appExit()
{
    pifLog_Clear();
    pif_Exit();
}
