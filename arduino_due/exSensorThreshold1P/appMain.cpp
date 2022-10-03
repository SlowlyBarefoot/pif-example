#include "appMain.h"
#include "exSensorThreshold1P.h"

#include "core/pif_log.h"
#include "filter/pif_noise_filter_uint16.h"
#include "sensor/pif_sensor_digital.h"


#define USE_FILTER_AVERAGE		1


PifTimerManager g_timer_1ms;


static void _evtSensorThreshold(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, void* p_issuer)
{
	(void)p_issuer;

	pifLog_Printf(LT_INFO, "Sensor: DC:%u S:%u V:%u", p_owner->_id, state, *(uint16_t*)p_value);
}

void appSetup()
{
	static PifComm s_comm_log;
#if USE_FILTER_AVERAGE
    static PifNoiseFilterUint16 s_filter;
#endif
    static PifSensorDigital s_sensor;

	pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;					// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;					// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

#if USE_FILTER_AVERAGE
	if (!pifNoiseFilterUint16_Init(&s_filter, 7)) return;
#endif

    if (!pifSensorDigital_Init(&s_sensor, PIF_ID_AUTO, actSensorAcquisition, NULL)) return;
    pifSensorDigital_SetThreshold(&s_sensor, 550, 550);
    if (!pifSensorDigital_AttachTaskAcquire(&s_sensor, TM_PERIOD_MS, 100, TRUE)) return;	// 100ms
#if USE_FILTER_AVERAGE
    s_sensor.p_filter = &s_filter.parent;
#endif
    pifSensor_AttachEvtChange(&s_sensor.parent, _evtSensorThreshold);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
