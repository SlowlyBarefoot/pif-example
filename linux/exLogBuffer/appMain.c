#include "appMain.h"
#include "main.h"

#include "pifLog.h"


BOOL appInit()
{
    pif_Init(NULL);

    if (!pifLog_InitHeap(0x200)) return FALSE;
    pifLog_AttachActPrint(actLogPrint);

	pifLog_Printf(LT_enInfo, "Start");

    pifLog_Disable();
    return TRUE;
}

void appExit()
{
	pifLog_Exit();
}
