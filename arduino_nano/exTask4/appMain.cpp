#include "appMain.h"
#include "exTask4.h"


#define TASK_COUNT              3


void appSetup()
{
	pif_Init();

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;			// 500ms
    if (!pifTask_AddRatio(100, taskLedRedToggle, NULL)) return;			// 100%
    if (!pifTask_AddPeriodUs(500, taskLedYellowToggle, NULL)) return;	// 500us
}
