#include "appMain.h"

#include "filter/pif_noise_filter_uint16.h"


#define USE_FILTER_AVERAGE		1


PifSensorDigital g_sensor;
PifTimerManager g_timer_1ms;


static void _evtTimerPeriodFinish(PifIssuerP p_issuer)
{
    PifSensorDigital* p_owner = (PifSensorDigital*)p_issuer;

	pifLog_Printf(LT_INFO, "Sensor: DC:%u L:%u", p_owner->parent._id, p_owner->__curr_level);
}

BOOL appSetup()
{
#if USE_FILTER_AVERAGE
    static PifNoiseFilterUint16 s_filter;
#endif
    PifTimer* p_timer;

#if USE_FILTER_AVERAGE
	if (!pifNoiseFilterUint16_Init(&s_filter, 7)) return FALSE;
#endif

    if (!pifSensorDigital_AttachTaskAcquire(&g_sensor, TM_PERIOD_MS, 50, TRUE)) return FALSE;		// 50ms
#if USE_FILTER_AVERAGE
    g_sensor.p_filter = &s_filter.parent;
#endif

	p_timer = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!p_timer) return FALSE;
    pifTimer_AttachEvtFinish(p_timer, _evtTimerPeriodFinish, &g_sensor);
	if (!pifTimer_Start(p_timer, 500)) return FALSE;												// 500ms
	return TRUE;
}
