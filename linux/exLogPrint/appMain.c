#include "appMain.h"
#include "main.h"

#include "pifLog.h"


BOOL appInit()
{
    pif_Init();

    pifLog_Init();
    pifLog_AttachActPrint(actLogPrint);
    return TRUE;
}

void appExit()
{
    pifLog_Exit();
}
