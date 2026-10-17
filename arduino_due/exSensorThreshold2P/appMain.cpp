#include "appMain.h"

#include "filter/pif_noise_filter_int32.h"


#define USE_FILTER_AVERAGE		1


PifSensorThreshold g_sensor;
PifTimerManager g_timer_1ms;


static void _evtSensorThreshold(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_issuer;

	pifLog_Printf(LT_INFO, "Sensor: DC:%u S:%u V:%ld", p_owner->_id, state, (long)*(int32_t*)p_value);
}

BOOL appSetup()
{
#if USE_FILTER_AVERAGE
    static PifNoiseFilterManager s_filter;
#endif

#if USE_FILTER_AVERAGE
    if (!pifNoiseFilterManager_Init(&s_filter, 1)) return FALSE;
#endif

    if (!pifSensorThreshold_SetThreshold(&g_sensor, 300, 600)) return FALSE;
    if (!pifSensorThreshold_AttachTaskAcquire(&g_sensor, PIF_ID_AUTO, TM_PERIOD, 100000, TRUE)) return FALSE;	// 100ms
#if USE_FILTER_AVERAGE
    g_sensor.p_filter = pifNoiseFilterInt32_AddAverage(&s_filter, 7);
    if (!g_sensor.p_filter) return FALSE;
#endif
    pifSensor_AttachEvtChange(&g_sensor.parent, _evtSensorThreshold, NULL);
    return TRUE;
}
