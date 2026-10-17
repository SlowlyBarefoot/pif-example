#include "appMain.h"

#include "filter/pif_noise_filter_int32.h"


#define USE_FILTER_AVERAGE		1


PifSensorThreshold g_sensor;
PifTimerManager g_timer_1ms;


static void _evtTimerPeriodFinish(PifIssuerP p_issuer)
{
    PifSensorThreshold* p_owner = (PifSensorThreshold*)p_issuer;

	pifLog_Printf(LT_INFO, "Sensor: DC:%u L:%ld", p_owner->parent._id, (long)p_owner->_curr_level);
}

BOOL appSetup()
{
#if USE_FILTER_AVERAGE
    static PifNfInt32Average s_filter;
    static int32_t s_filter_buffer[7];
#endif
    PifTimer* p_timer;

    if (!pifSensorThreshold_AttachTaskAcquire(&g_sensor, PIF_ID_AUTO, TM_PERIOD, 50000, TRUE)) return FALSE;	// 50ms
#if USE_FILTER_AVERAGE
    if (!pifNoiseFilterInt32_InitAverage(&s_filter, s_filter_buffer, 7)) return FALSE;
    if (!pifSensorThreshold_AttachFilter(&g_sensor, &s_filter.parent)) return FALSE;
#endif

	p_timer = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!p_timer) return FALSE;
    pifTimer_AttachEvtFinish(p_timer, _evtTimerPeriodFinish, &g_sensor, FALSE);
	if (!pifTimer_Start(p_timer, 500)) return FALSE;														// 500ms
	return TRUE;
}
