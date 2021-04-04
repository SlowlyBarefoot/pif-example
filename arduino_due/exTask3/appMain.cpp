#include "appMain.h"
#include "exTask3.h"

#include "pifTask.h"


#define TASK_COUNT              3


static PIF_stTask *s_pstTask[TASK_COUNT];


static void _taskLedRedToggle(PIF_stTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	actLedRed(sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->bPause = TRUE;
			s_pstTask[1]->bPause = FALSE;
		}

		nCount = 9999;
    }
    else nCount--;
}

static void _taskLedYellowToggle(PIF_stTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	actLedYellow(sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->bPause = TRUE;
			s_pstTask[2]->bPause = FALSE;
		}

		nCount = 9999;
    }
    else nCount--;
}

static void _taskLedGreenToggle(PIF_stTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	actLedGreen(sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->bPause = TRUE;
			s_pstTask[0]->bPause = FALSE;
		}

		nCount = 9999;
    }
    else nCount--;
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTask_Init(TASK_COUNT)) return;

    s_pstTask[0] = pifTask_AddRatio(30, _taskLedRedToggle, NULL);		// 30%
    if (!s_pstTask[0]) return;

    s_pstTask[1] = pifTask_AddRatio(60, _taskLedYellowToggle, NULL);	// 60%
    if (!s_pstTask[1]) return;
    s_pstTask[1]->bPause = TRUE;

    s_pstTask[2] = pifTask_AddRatio(100, _taskLedGreenToggle, NULL);	// 100%
    if (!s_pstTask[2]) return;
    s_pstTask[2]->bPause = TRUE;
}
