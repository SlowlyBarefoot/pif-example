#include "app_port.h"


BOOL appSetup()
{
	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***              exTask5             ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);

#ifdef PIF_DEBUG
	pifTaskManager_PrintRatioTable();
#endif
	return TRUE;
}
