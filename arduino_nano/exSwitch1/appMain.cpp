#include "appMain.h"
#include "exSwitch1.h"

#include "pifLog.h"
#include "pifSensorSwitch.h"


#define SWITCH_COUNT            2
#define TASK_COUNT              2


static PIF_stSensor *s_pstPushSwitch = NULL;
static PIF_stSensor *s_pstTiltSwitch = NULL;


void appSetup()
{
    pif_Init(NULL);

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;

    s_pstPushSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, OFF);
    if (!s_pstPushSwitch) return;
    pifSensor_AttachAction(s_pstPushSwitch, actPushSwitchAcquire);
    pifSensor_AttachEvtChange(s_pstPushSwitch, evtPushSwitchChange, NULL);

    s_pstTiltSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, OFF);
	if (!s_pstTiltSwitch) return;
	pifSensor_AttachAction(s_pstTiltSwitch, actTiltSwitchAcquire);
	pifSensor_AttachEvtChange(s_pstTiltSwitch, evtTiltSwitchChange, NULL);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(3, pifSensorSwitch_taskAll, NULL)) return;		// 3%

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;				// 500ms
}
