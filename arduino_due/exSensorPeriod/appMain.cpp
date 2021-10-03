#include "appMain.h"
#include "exSensorPeriod.h"

#include "pifLog.h"


#define USE_FILTER_AVERAGE		0


PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stSensor *g_pstSensor = NULL;

#if USE_FILTER_AVERAGE
static PIF_stSensorDigitalFilter s_stFilter;
#endif


static void _evtSensorPeriod(PIF_usId usPifId, uint16_t usLevel)
{
	pifLog_Printf(LT_enInfo, "Sensor: DC:%u L:%u", usPifId, usLevel);
}

void appSetup()
{
	PIF_stComm *pstCommLog;

	pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);										// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;					// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return;					// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstSensor = pifSensorDigital_Create(PIF_ID_AUTO, g_pstTimer1ms);
    if (!g_pstSensor) return;
    if (!pifSensorDigital_AttachTask(g_pstSensor, TM_enRatio, 3, TRUE)) return;				// 3%
#if USE_FILTER_AVERAGE
    pifSensorDigital_AttachFilter(g_pstSensor, PIF_SENSOR_DIGITAL_FILTER_AVERAGE, 7, &s_stFilter, TRUE);
#endif
    if (!pifSensorDigital_AttachEvtPeriod(g_pstSensor, _evtSensorPeriod)) return;

    if (!pifTaskManager_Add(TM_enPeriodMs, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms
    if (!pifTaskManager_Add(TM_enPeriodMs, 100, taskSensorAcquisition, NULL, TRUE)) return;	// 100ms

    if (!pifSensorDigital_StartPeriod(g_pstSensor, 500)) return;							// 500ms
}
