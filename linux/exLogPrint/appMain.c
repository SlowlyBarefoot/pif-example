#include "appMain.h"
#include "main.h"

#include "pif_log.h"


BOOL appInit()
{
	PifComm *pstCommLog;

    pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return FALSE;

    pifLog_Init();

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return FALSE;
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return FALSE;

    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
    return TRUE;
}

void appExit()
{
    pifLog_Clear();
    pif_Exit();
}
