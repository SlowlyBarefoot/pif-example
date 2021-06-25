#include "appMain.h"
#include "exTask1.h"

#include "pifTask.h"


#define TASK_COUNT              1


static uint16_t _taskLedToggle(PIF_stTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

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

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(50, _taskLedToggle, NULL)) return;	// 50%
}
