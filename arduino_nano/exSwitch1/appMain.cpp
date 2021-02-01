#include "appMain.h"
#include "exSwitch1.h"

#include "pifLog.h"
#include "pifSwitch.h"


#define SWITCH_COUNT            2
#define TASK_COUNT              2


static PIF_stSwitch *s_pstPushSwitch = NULL;
static PIF_stSwitch *s_pstTiltSwitch = NULL;


void appSetup()
{
    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifSwitch_Init(SWITCH_COUNT)) return;

    s_pstPushSwitch = pifSwitch_Add(PIF_ID_AUTO, 0);
    if (!s_pstPushSwitch) return;
    s_pstPushSwitch->bStateReverse = TRUE;
    pifSwitch_AttachAction(s_pstPushSwitch, actPushSwitchAcquire);
    pifSwitch_AttachEvtChange(s_pstPushSwitch, evtPushSwitchChange, NULL);

    s_pstTiltSwitch = pifSwitch_Add(PIF_ID_AUTO, 0);
	if (!s_pstTiltSwitch) return;
	pifSwitch_AttachAction(s_pstTiltSwitch, actTiltSwitchAcquire);
	pifSwitch_AttachEvtChange(s_pstTiltSwitch, evtTiltSwitchChange, NULL);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(3, pifSwitch_taskAll, NULL)) return;		// 3%

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;		// 500ms
}
