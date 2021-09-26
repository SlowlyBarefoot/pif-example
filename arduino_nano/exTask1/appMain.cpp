#include "appMain.h"
#include "exTask1.h"

#include "pifTask.h"


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

    if (!pifTaskManager_Add(TM_enRatio, 50, _taskLedToggle, NULL, TRUE)) return;	// 50%
}
