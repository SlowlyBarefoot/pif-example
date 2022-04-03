#include "appMain.h"
#include "exTask4.h"


void appSetup(PifActTimer1us act_timer1us)
{
	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms
    if (!pifTaskManager_Add(TM_ALWAYS, 100, taskLedRedToggle, NULL, TRUE)) return;			// 100%
    if (!pifTaskManager_Add(TM_PERIOD_US, 200, taskLedYellowToggle, NULL, TRUE)) return;	// 200us
}
