#include "appMain.h"
#include "exTask2.h"

#include "pifTask.h"


#define TASK_COUNT              3


static uint16_t _taskLedRedToggle(PIF_stTask *pstTask)
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

static uint16_t _taskLedYellowToggle(PIF_stTask *pstTask)
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

static uint16_t _taskLedGreenToggle(PIF_stTask *pstTask)
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

    if (!pifTask_Init(TASK_COUNT)) return;

    if (!pifTask_Add(TM_enRatio, 30, _taskLedRedToggle, NULL, TRUE)) return;		// 30%
    if (!pifTask_Add(TM_enRatio, 60, _taskLedYellowToggle, NULL, TRUE)) return;		// 60%
    if (!pifTask_Add(TM_enRatio, 100, _taskLedGreenToggle, NULL, TRUE)) return;		// 100%
}
