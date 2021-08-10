#include "appMain.h"
#include "exTerminal.h"

#include "pifLog.h"
#include "pifTerminal.h"


#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              3


const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },

	{ NULL, NULL, NULL }
};


void appSetup()
{
	PIF_stComm *pstComm;

    pif_Init(NULL);

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;
    pstComm = pifComm_Add(PIF_ID_AUTO);
	if (!pstComm) return;
	pifComm_AttachActReceiveData(pstComm, actSerialReceiveData);
	pifComm_AttachActSendData(pstComm, actSerialSendData);

    if (!pifTerminal_Init(c_psCmdTable, "\nDebug")) return;
	pifTerminal_AttachComm(pstComm);

	pifLog_DetachActPrint();
    pifLog_UseTerminal(TRUE);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriodMs(10, pifComm_taskAll, NULL)) return;		// 10ms

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;			// 500ms
}
