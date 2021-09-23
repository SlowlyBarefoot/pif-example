#include "appMain.h"
#include "exSensorThreshold2P.h"

#include "pifLog.h"


#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define SENSOR_COUNT         	1
#define TASK_COUNT              5

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
	PIF_stComm *pstCommLog;

	pif_Init(NULL);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);					// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;				// 100%

    if (!pifSensorDigital_Init(SENSOR_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return;				// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstSensor = pifSensorDigital_Add(PIF_ID_AUTO);
    if (!g_pstSensor) return;
    if (!pifSensorDigital_AttachTask(g_pstSensor, TM_enPeriodMs, 100, TRUE)) return;	// 100ms
#if USE_FILTER_AVERAGE
    pifSensorDigital_AttachFilter(g_pstSensor, PIF_SENSOR_DIGITAL_FILTER_AVERAGE, 7, &s_stFilter, TRUE);
#endif
    pifSensorDigital_SetEventThreshold2P(g_pstSensor, 200, 300);
    pifSensor_AttachEvtChange(g_pstSensor, _evtSensorThreshold, NULL);

    if (!pifTask_Add(TM_enPeriodMs, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms
    if (!pifTask_Add(TM_enPeriodMs, 100, taskSensorAcquisition, NULL, TRUE)) return;	// 100ms
}
