#include "appMain.h"
#include "exTask3.h"

#include "pifTask.h"


#define TASK_COUNT              3


static PIF_stTask *s_pstTask[TASK_COUNT];


static uint16_t _taskLedRedToggle(PIF_stTask *pstTask)
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
    return 0;
}

static uint16_t _taskLedYellowToggle(PIF_stTask *pstTask)
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
    return 0;
}

static uint16_t _taskLedGreenToggle(PIF_stTask *pstTask)
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
    return 0;
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTask_Init(TASK_COUNT)) return;

    s_pstTask[0] = pifTask_Add(TM_enRatio, 30, _taskLedRedToggle, NULL, TRUE);		// 30%
    if (!s_pstTask[0]) return;

    s_pstTask[1] = pifTask_Add(TM_enRatio, 60, _taskLedYellowToggle, NULL, FALSE);	// 60%
    if (!s_pstTask[1]) return;

    s_pstTask[2] = pifTask_Add(TM_enRatio, 100, _taskLedGreenToggle, NULL, FALSE);	// 100%
    if (!s_pstTask[2]) return;
}
