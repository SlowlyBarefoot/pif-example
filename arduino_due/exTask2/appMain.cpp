#include "appMain.h"
#include "exTask2.h"

#include "pifTask.h"


static uint16_t _taskLedRedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	actLedRed(sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

static uint16_t _taskLedYellowToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	actLedYellow(sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

static uint16_t _taskLedGreenToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

	(void)pstTask;

    if (!nCount) {
    	actLedGreen(sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Add(TM_RATIO, 30, _taskLedRedToggle, NULL, TRUE)) return;		// 30%
    if (!pifTaskManager_Add(TM_RATIO, 60, _taskLedYellowToggle, NULL, TRUE)) return;	// 60%
    if (!pifTaskManager_Add(TM_RATIO, 100, _taskLedGreenToggle, NULL, TRUE)) return;	// 100%
}
