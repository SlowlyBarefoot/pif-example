#include "appMain.h"
#include "main.h"

#include "pifLog.h"


BOOL appInit()
{
	PifComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return FALSE;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return FALSE;

    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms
    return TRUE;
}

void appExit()
{
    pifLog_Clear();
    pif_Exit();
}
