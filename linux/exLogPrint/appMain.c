#include "appMain.h"
#include "main.h"

#include "pifLog.h"


#define COMM_COUNT         		1
#define TASK_COUNT              1


BOOL appInit()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);

    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return FALSE;
    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return FALSE;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return FALSE;

    if (!pifTask_Init(TASK_COUNT)) return FALSE;
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return FALSE;				// 1ms
    return TRUE;
}

void appExit()
{
    pifLog_Exit();
}
