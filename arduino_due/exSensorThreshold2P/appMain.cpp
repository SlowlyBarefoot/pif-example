#include "appMain.h"


#define USE_FILTER_AVERAGE		1


PifSensorDigital g_sensor;
PifTimerManager g_timer_1ms;


static void _evtSensorThreshold(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_issuer;

	pifLog_Printf(LT_INFO, "Sensor: DC:%u S:%u V:%u", p_owner->_id, state, *(uint16_t*)p_value);
}

BOOL appSetup()
{
#if USE_FILTER_AVERAGE
    static PifNoiseFilterUint16 s_filter;
#endif

#if USE_FILTER_AVERAGE
    if (!pifNoiseFilterUint16_Init(&s_filter, 7)) return FALSE;
#endif

    pifSensorDigital_SetThreshold(&g_sensor, 300, 600);
    if (!pifSensorDigital_AttachTaskAcquire(&g_sensor, TM_PERIOD_MS, 100, TRUE)) return FALSE;	// 100ms
#if USE_FILTER_AVERAGE
    g_sensor.p_filter = &s_filter.parent;
#endif
    g_sensor.parent.evt_change = _evtSensorThreshold;
    return TRUE;
}
