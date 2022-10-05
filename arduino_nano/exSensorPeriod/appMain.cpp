#include "appMain.h"
#include "exSensorPeriod.h"

#include "core/pif_log.h"
#include "filter/pif_noise_filter_uint16.h"
#include "sensor/pif_sensor_digital.h"


#define USE_FILTER_AVERAGE		1


PifTimerManager g_timer_1ms;


static void _evtTimerPeriodFinish(PifIssuerP p_issuer)
{
    PifSensorDigital* p_owner = (PifSensorDigital*)p_issuer;

	pifLog_Printf(LT_INFO, "Sensor: DC:%u L:%u", p_owner->parent._id, p_owner->__curr_level);
}

void appSetup()
{
	static PifComm s_comm_log;
#if USE_FILTER_AVERAGE
    static PifNoiseFilterUint16 s_filter;
#endif
    static PifSensorDigital s_sensor;
    PifTimer* p_timer;

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
    if (!pifSensorDigital_AttachTaskAcquire(&s_sensor, TM_PERIOD_MS, 50, TRUE)) return;		// 50ms
#if USE_FILTER_AVERAGE
    s_sensor.p_filter = &s_filter.parent;
#endif

	p_timer = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!p_timer) return;
    pifTimer_AttachEvtFinish(p_timer, _evtTimerPeriodFinish, &s_sensor);
	if (!pifTimer_Start(p_timer, 500)) return;												// 500ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
