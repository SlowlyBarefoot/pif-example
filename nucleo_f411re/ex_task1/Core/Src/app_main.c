#include "app_main.h"
#include "main.h"

#include "pif_task.h"


static uint16_t _taskLedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = OFF;

	(void)pstTask;

    if (!nCount) {
    	actLedL(sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(1)) return;

	if (!pifTaskManager_Add(TM_RATIO, 50, _taskLedToggle, NULL, TRUE)) return;	// 50%
}
