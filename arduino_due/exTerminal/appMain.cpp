#include "appMain.h"
#include "exTerminal.h"

#include "pifLog.h"
#include "pifTerminal.h"


#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              3


PIF_stComm *g_pstComm = NULL;

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },

	{ NULL, NULL, NULL }
};


void appSetup()
{
    pif_Init(NULL);

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;
	g_pstComm = pifComm_Add(0);
	if (!g_pstComm) return;
	pifComm_AttachActReceiveData(g_pstComm, actSerialReceiveData);
	pifComm_AttachActSendData(g_pstComm, actSerialSendData);

    if (!pifTerminal_Init(c_psCmdTable, "\nDebug")) return;
	pifTerminal_AttachComm(g_pstComm);

	pifLog_DetachActPrint();
    pifLog_UseTerminal(TRUE);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriodMs(10, pifComm_taskAll, NULL)) return;		// 10ms

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;			// 500ms
}
