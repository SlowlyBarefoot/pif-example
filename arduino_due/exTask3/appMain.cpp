#include "appMain.h"
#include "exTask3.h"

#include "core/pif_task.h"


static PifTask *s_pstTask[3];


static uint16_t _taskLedRedToggle(PifTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	actLedRed(sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->pause = TRUE;
			s_pstTask[1]->pause = FALSE;
		}

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

static uint16_t _taskLedYellowToggle(PifTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	actLedYellow(sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->pause = TRUE;
			s_pstTask[2]->pause = FALSE;
		}

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

static uint16_t _taskLedGreenToggle(PifTask *pstTask)
{
	static int nCount = 0, nToggle = 0;
	static BOOL sw = LOW;

    if (!nCount) {
    	actLedGreen(sw);
		sw ^= 1;

		nToggle++;
		if (nToggle >= 10) {
			nToggle = 0;
			pstTask->pause = TRUE;
			s_pstTask[0]->pause = FALSE;
		}

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    s_pstTask[0] = pifTaskManager_Add(TM_RATIO, 30, _taskLedRedToggle, NULL, TRUE);		// 30%
    if (!s_pstTask[0]) return;

    s_pstTask[1] = pifTaskManager_Add(TM_RATIO, 60, _taskLedYellowToggle, NULL, FALSE);	// 60%
    if (!s_pstTask[1]) return;

    s_pstTask[2] = pifTaskManager_Add(TM_ALWAYS, 100, _taskLedGreenToggle, NULL, FALSE);	// 100%
    if (!s_pstTask[2]) return;
}
