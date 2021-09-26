#include "appMain.h"
#include "exSwitch1.h"

#include "pifLog.h"
#include "pifSensorSwitch.h"


#define SWITCH_COUNT            2


static PIF_stSensor *s_pstPushSwitch = NULL;
static PIF_stSensor *s_pstTiltSwitch = NULL;


void appSetup()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;

    pstCommLog = pifComm_Init(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return;			// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstPushSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, OFF);
    if (!s_pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(s_pstPushSwitch, TM_enRatio, 3, TRUE)) return;	// 3%
    pifSensor_AttachAction(s_pstPushSwitch, actPushSwitchAcquire);
    pifSensor_AttachEvtChange(s_pstPushSwitch, evtPushSwitchChange, NULL);

    s_pstTiltSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, OFF);
	if (!s_pstTiltSwitch) return;
    if (!pifSensorSwitch_AttachTask(s_pstTiltSwitch, TM_enRatio, 3, TRUE)) return;	// 3%
	pifSensor_AttachAction(s_pstTiltSwitch, actTiltSwitchAcquire);
	pifSensor_AttachEvtChange(s_pstTiltSwitch, evtTiltSwitchChange, NULL);

    if (!pifTaskManager_Add(TM_enPeriodMs, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
}
