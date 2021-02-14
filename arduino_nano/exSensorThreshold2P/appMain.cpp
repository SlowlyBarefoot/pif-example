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
static PIF_stSensorDigitalFilter s_stFilter;
#endif


static void _evtSensorThreshold(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)pvIssuer;

	pifLog_Printf(LT_enInfo, "Sensor: DC:%u SW:%u", usPifId, usLevel);
}

void appSetup()
{
	pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifSensorDigital_Init(g_pstTimer1ms, SENSOR_COUNT)) return;
    g_pstSensor = pifSensorDigital_Add(PIF_ID_AUTO);
    if (!g_pstSensor) return;
#if USE_FILTER_AVERAGE
    pifSensorDigital_AttachFilter(g_pstSensor, PIF_SENSOR_DIGITAL_FILTER_AVERAGE, 7, &s_stFilter, TRUE);
#endif
    pifSensorDigital_SetEventThreshold2P(g_pstSensor, 200, 300);
    pifSensor_AttachEvtChange(g_pstSensor, _evtSensorThreshold, NULL);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddPeriodMs(100, pifSensorDigital_taskAll, NULL)) return;	// 100ms

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;				// 500ms
    if (!pifTask_AddPeriodMs(100, taskSensorAcquisition, NULL)) return;		// 100ms
}
