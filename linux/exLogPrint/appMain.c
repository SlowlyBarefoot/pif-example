#include "appMain.h"
#include "main.h"

#include "pifLog.h"


#define TASK_COUNT              1


BOOL appInit()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifTask_Init(TASK_COUNT)) return FALSE;

    pstCommLog = pifComm_Init(PIF_ID_AUTO);
	if (!pstCommLog) return FALSE;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return FALSE;

    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return FALSE;	// 1ms
    return TRUE;
}

void appExit()
{
    pifLog_Exit();
}
