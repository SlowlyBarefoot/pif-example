#include "appMain.h"
#include "exSwitch1.h"

#include "pif_log.h"
#include "pif_sensor_switch.h"


static PifSensor *s_pstPushSwitch = NULL;
static PifSensor *s_pstTiltSwitch = NULL;


void appSetup()
{
	static PifComm s_comm_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;		   	// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

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
