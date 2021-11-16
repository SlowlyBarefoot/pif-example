#include "appMain.h"
#include "exSwitch2.h"

#include "pif_led.h"
#include "pif_log.h"


PifSensor *g_pstPushSwitch = NULL;
PifSensor *g_pstTiltSwitch = NULL;
PifTimerManager g_timer_1ms;

static PifLed s_led_l;


void appSetup()
{
	static PifComm s_comm_log;
	PifTimer *pstTimerSwitch;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;									// 500ms

    pstTimerSwitch = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimerSwitch) return;
    pifTimer_AttachEvtFinish(pstTimerSwitch, evtSwitchAcquire, NULL);

    g_pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
    if (!g_pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(g_pstPushSwitch, TM_RATIO, 3, TRUE)) return;	// 3%
    pifSensor_AttachEvtChange(g_pstPushSwitch, evtPushSwitchChange, NULL);

    g_pstTiltSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
	if (!g_pstTiltSwitch) return;
    if (!pifSensorSwitch_AttachTask(g_pstTiltSwitch, TM_RATIO, 3, TRUE)) return;	// 3%
	pifSensor_AttachEvtChange(g_pstTiltSwitch, evtTiltSwitchChange, NULL);

    pifLed_BlinkOn(&s_led_l, 0);

    pifTimer_Start(pstTimerSwitch, 20);										    	// 20ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
