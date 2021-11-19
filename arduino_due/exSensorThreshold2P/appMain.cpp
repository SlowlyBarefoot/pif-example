#include "appMain.h"
#include "exSensorThreshold2P.h"

#include "pif_log.h"


#define USE_FILTER_AVERAGE		0


PifTimerManager g_timer_1ms;
PifSensorDigital g_sensor;

#if USE_FILTER_AVERAGE
static PIF_stSensorDigitalFilter s_stFilter;
#endif


static void _evtSensorThreshold(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)pvIssuer;

	pifLog_Printf(LT_INFO, "Sensor: DC:%u SW:%u", usPifId, usLevel);
}

void appSetup()
{
	static PifComm s_comm_log;

	pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;					// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;					// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifSensorDigital_Init(&g_sensor, PIF_ID_AUTO, &g_timer_1ms)) return;
    if (!pifSensorDigital_AttachTask(&g_sensor, TM_PERIOD_MS, 100, TRUE)) return;			// 100ms
#if USE_FILTER_AVERAGE
    pifSensorDigital_AttachFilter(&g_sensor, PIF_SENSOR_DIGITAL_FILTER_AVERAGE, 7, &s_stFilter, TRUE);
#endif
    pifSensorDigital_SetEventThreshold2P(&g_sensor, 400, 700);
    pifSensor_AttachEvtChange(&g_sensor.parent, _evtSensorThreshold, NULL);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 100, taskSensorAcquisition, NULL, TRUE)) return;	// 100ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
