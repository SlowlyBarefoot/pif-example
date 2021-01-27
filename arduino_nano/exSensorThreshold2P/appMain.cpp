#include "appMain.h"
#include "exSensorThreshold2P.h"

#include "pifLog.h"


#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define SENSOR_COUNT         	1
#define TASK_COUNT              4

#define USE_FILTER_AVERAGE		0


PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stSensor *g_pstSensor = NULL;

#if USE_FILTER_AVERAGE
static PIF_stSensorFilter s_stFilter;
#endif


static void _evtSensorThreshold(PIF_usId usPifId, SWITCH swState)
{
	pifLog_Printf(LT_enInfo, "Sensor: DC:%u SW:%u", usPifId, swState);
}

void appSetup()
{
	pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifSensor_Init(g_pstTimer1ms, SENSOR_COUNT)) return;
    g_pstSensor = pifSensor_Add(PIF_ID_AUTO);
    if (!g_pstSensor) return;
#if USE_FILTER_AVERAGE
    pifSensor_AttachFilter(g_pstSensor, PIF_SENSOR_FILTER_AVERAGE, 7, &s_stFilter, TRUE);
#endif
    pifSensor_SetEventThreshold2P(g_pstSensor, 100, 200, _evtSensorThreshold);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddPeriodMs(100, pifSensor_taskAll, NULL)) return;			// 100ms

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;				// 500ms
    if (!pifTask_AddPeriodMs(100, taskSensorAcquisition, NULL)) return;		// 100ms
}
