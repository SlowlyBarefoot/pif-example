#include "appMain.h"
#include "exGpsSerialNmea.h"

#include "pifGpsNmea.h"
#include "pifLed.h"
#include "pifLog.h"
#include "pifSensorSwitch.h"


PifPulse *g_pstTimer1ms = NULL;
BOOL g_bPrintRawData = FALSE;

static PIF_stComm *s_pstCommGps = NULL;
static PIF_stGpsNmea *s_pstGpsNmea = NULL;
static PIF_stLed *s_pstLedL = NULL;


static void _evtGpsReceive(PIF_stGps *pstOwner)
{
	PIF_stDegMin stLatDegMin, stLonDegMin;
	PIF_stDegMinSec stLatDegMinSec, stLonDegMinSec;

	pifLed_EachToggle(s_pstLedL, 1);

	pifGps_ConvertLatitude2DegMin(pstOwner, &stLatDegMin);
	pifGps_ConvertLongitude2DegMin(pstOwner, &stLonDegMin);

	pifGps_ConvertLatitude2DegMinSec(pstOwner, &stLatDegMinSec);
	pifGps_ConvertLongitude2DegMinSec(pstOwner, &stLonDegMinSec);

	if (!g_bPrintRawData) {
		pifLog_Printf(LT_enInfo, "UTC Date Time: %4u/%2u/%2u %2u:%2u:%2u.%3u",
				2000 + pstOwner->_stDateTime.year, pstOwner->_stDateTime.month, pstOwner->_stDateTime.day,
				pstOwner->_stDateTime.hour, pstOwner->_stDateTime.minute, pstOwner->_stDateTime.second, pstOwner->_stDateTime.millisecond);
		pifLog_Printf(LT_enInfo, "Longitude: %f` - %u`%f' - %u`%u'%f\"",
				pstOwner->_dCoordDeg[GPS_LON], stLonDegMin.usDegree, stLonDegMin.dMinute,
				stLonDegMinSec.usDegree, stLonDegMinSec.usMinute, stLonDegMinSec.dSecond);
		pifLog_Printf(LT_enInfo, "Latitude: %f` - %u`%f' - %u`%u'%f\"",
				pstOwner->_dCoordDeg[GPS_LAT], stLatDegMin.usDegree, stLatDegMin.dMinute,
				stLatDegMinSec.usDegree, stLatDegMinSec.usMinute, stLatDegMinSec.dSecond);
		pifLog_Printf(LT_enInfo, "NumSat: %u", pstOwner->_ucNumSat);
		pifLog_Printf(LT_enInfo, "Altitude: %f m", pstOwner->_dAltitude);
		pifLog_Printf(LT_enInfo, "Speed: %f knots %f m/s %f km/h", pstOwner->_dSpeedN, pifGps_ConvertKnots2MpS(pstOwner->_dSpeedN), pstOwner->_dSpeedK);
		pifLog_Printf(LT_enInfo, "Ground Course: %f deg", pstOwner->_dGroundCourse);
		pifLog_Printf(LT_enInfo, "Fix: %u", pstOwner->_ucFix);
	}
	pifLog_Printf(LT_enNone, "\n");
}

static void _evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	if (usLevel) {
		g_bPrintRawData ^= 1;
	}
}

void appSetup()
{
	PIF_stComm *pstCommLog;
	PIF_stSensor *pstPushSwitch;

	pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);									// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;				// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 2, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;									// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

	pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, 0);
	if (!pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(pstPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;	// 10m
	pifSensor_AttachAction(pstPushSwitch, actPushSwitchAcquire);
	pifSensor_AttachEvtChange(pstPushSwitch, _evtPushSwitchChange, NULL);

	s_pstCommGps = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstCommGps) return;
    if (!pifComm_AttachTask(s_pstCommGps, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pifComm_AttachActReceiveData(s_pstCommGps, actGpsReceiveData);
	pifComm_AttachActSendData(s_pstCommGps, actGpsSendData);

	s_pstGpsNmea = pifGpsNmea_Create(PIF_ID_AUTO);
	if (!s_pstGpsNmea) return;
	if (!pifGpsNmea_SetProcessMessageId(s_pstGpsNmea, 2, NMEA_MESSAGE_ID_GGA, NMEA_MESSAGE_ID_VTG)) return;
	pifGpsNmea_SetEventMessageId(s_pstGpsNmea, NMEA_MESSAGE_ID_GGA);
	pifGpsNmea_AttachComm(s_pstGpsNmea, s_pstCommGps);
	pifGps_AttachEvent(&s_pstGpsNmea->_stGps, _evtGpsReceive);
}
