#include "appMain.h"
#include "exSensorThreshold2P.h"

#include "pif_log.h"


#define USE_FILTER_AVERAGE		0


PifTimerManager *g_pstTimer1ms = NULL;
PifSensor *g_pstSensor = NULL;

#if USE_FILTER_AVERAGE
static PIF_stSensorDigitalFilter s_stFilter;
#endif


static void _evtSensorThreshold(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)pvIssuer;

	pifLog_Printf(LT_INFO, "Sensor: DC:%u SW:%u", usPifId, usLevel);
}

void appSetup()
{
	PifComm *pstCommLog;

	pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 1);							// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;						// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstSensor = pifSensorDigital_Create(PIF_ID_AUTO, g_pstTimer1ms);
    if (!g_pstSensor) return;
    if (!pifSensorDigital_AttachTask(g_pstSensor, TM_PERIOD_MS, 100, TRUE)) return;			// 100ms
#if USE_FILTER_AVERAGE
    pifSensorDigital_AttachFilter(g_pstSensor, PIF_SENSOR_DIGITAL_FILTER_AVERAGE, 7, &s_stFilter, TRUE);
#endif
    pifSensorDigital_SetEventThreshold2P(g_pstSensor, 200, 300);
    pifSensor_AttachEvtChange(g_pstSensor, _evtSensorThreshold, NULL);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 100, taskSensorAcquisition, NULL, TRUE)) return;	// 100ms

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(g_pstTimer1ms));
}
