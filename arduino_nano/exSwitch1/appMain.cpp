#include "appMain.h"
#include "exSwitch1.h"

#include "pif_log.h"
#include "pif_sensor_switch.h"


static PifSensor *s_pstPushSwitch = NULL;
static PifSensor *s_pstTiltSwitch = NULL;


void appSetup()
{
	PifComm *pstCommLog;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
    if (!s_pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(s_pstPushSwitch, TM_RATIO, 3, TRUE)) return;	// 3%
    pifSensor_AttachAction(s_pstPushSwitch, actPushSwitchAcquire);
    pifSensor_AttachEvtChange(s_pstPushSwitch, evtPushSwitchChange, NULL);

    s_pstTiltSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
	if (!s_pstTiltSwitch) return;
    if (!pifSensorSwitch_AttachTask(s_pstTiltSwitch, TM_RATIO, 3, TRUE)) return;	// 3%
	pifSensor_AttachAction(s_pstTiltSwitch, actTiltSwitchAcquire);
	pifSensor_AttachEvtChange(s_pstTiltSwitch, evtTiltSwitchChange, NULL);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
}
