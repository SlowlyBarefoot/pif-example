#include "appMain.h"
#include "main.h"

#include "pifLog.h"


BOOL appInit()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return FALSE;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return FALSE;

    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return FALSE;	// 1ms
    return TRUE;
}

void appExit()
{
    pifLog_Clear();
}
