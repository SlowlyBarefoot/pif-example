#include "appMain.h"
#include "exTask4.h"


#define TASK_COUNT              3


void appSetup(PIF_actTimer1us actTimer1us)
{
	pif_Init(actTimer1us);

    if (!pifTask_Init(TASK_COUNT)) return;

    if (!pifTask_Add(TM_enPeriodMs, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms
    if (!pifTask_Add(TM_enRatio, 100, taskLedRedToggle, NULL, TRUE)) return;			// 100%
    if (!pifTask_Add(TM_enPeriodUs, 200, taskLedYellowToggle, NULL, TRUE)) return;		// 200us
}
