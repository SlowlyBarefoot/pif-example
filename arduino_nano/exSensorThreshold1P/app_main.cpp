#include "app_port.h"

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
	PifTask* p_task;
#if USE_FILTER_AVERAGE
    static PifNfInt32Average s_filter;
    static int32_t s_filter_buffer[7];
#endif

    if (!pifSensorThreshold_SetThreshold(&g_sensor, 200, 200)) return FALSE;
    p_task = pifSensorThreshold_AttachTaskAcquire(&g_sensor, PIF_ID_AUTO, TM_PERIOD, 100000, TRUE);	// 100ms
    if (!p_task) return FALSE;
    p_task->name = "Sensor";
#if USE_FILTER_AVERAGE
    if (!pifNoiseFilterInt32_InitAverage(&s_filter, s_filter_buffer, 7)) return FALSE;
    if (!pifSensorThreshold_AttachFilter(&g_sensor, &s_filter.parent)) return FALSE;
#endif
	pifSensor_AttachEvtChange(&g_sensor.parent, _evtSensorThreshold, NULL);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***       exSensorThreshold1P        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;
}
