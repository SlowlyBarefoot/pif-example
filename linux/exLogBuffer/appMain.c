#include "appMain.h"
#include "main.h"

#include "pifLog.h"


#define COMM_COUNT         		1
#define TASK_COUNT              1

#define LOG_BUFFER_SIZE			0x200


BOOL appInit()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    if (!pifLog_InitHeap(LOG_BUFFER_SIZE)) return FALSE;

    if (!pifTask_Init(TASK_COUNT)) return FALSE;

    if (!pifComm_Init(COMM_COUNT)) return FALSE;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return FALSE;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return FALSE;

    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return FALSE;	// 1ms

	pifLog_Printf(LT_enInfo, "Start");

    pifLog_Disable();
    return TRUE;
}

void appExit()
{
	pifLog_Exit();
}
