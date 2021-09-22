#include "appMain.h"
#include "exSwitch1.h"

#include "pifLog.h"
#include "pifSensorSwitch.h"


#define COMM_COUNT            	1
#define SWITCH_COUNT            2
#define TASK_COUNT              4


static PIF_stSensor *s_pstPushSwitch = NULL;
static PIF_stSensor *s_pstTiltSwitch = NULL;


void appSetup()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_Add(TM_enPeriodMs, 1, pifComm_Task, pstCommLog, TRUE)) return;				// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstPushSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, OFF);
    if (!s_pstPushSwitch) return;
    if (!pifTask_Add(TM_enRatio, 3, pifSensorSwitch_Task, s_pstPushSwitch, TRUE)) return;	// 3%
    pifSensor_AttachAction(s_pstPushSwitch, actPushSwitchAcquire);
    pifSensor_AttachEvtChange(s_pstPushSwitch, evtPushSwitchChange, NULL);

    s_pstTiltSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, OFF);
	if (!s_pstTiltSwitch) return;
    if (!pifTask_Add(TM_enRatio, 3, pifSensorSwitch_Task, s_pstTiltSwitch, TRUE)) return;	// 3%
	pifSensor_AttachAction(s_pstTiltSwitch, actTiltSwitchAcquire);
	pifSensor_AttachEvtChange(s_pstTiltSwitch, evtTiltSwitchChange, NULL);

    if (!pifTask_Add(TM_enPeriodMs, 500, taskLedToggle, NULL, TRUE)) return;				// 500ms
}
