#include "appMain.h"
#include "exGpsSerialNmea.h"

#include "pif_gps_nmea.h"
#include "pif_led.h"
#include "pif_log.h"
#include "pif_sensor_switch.h"


PifPulse *g_pstTimer1ms = NULL;
BOOL g_bPrintRawData = FALSE;

static PifComm *s_pstCommGps = NULL;
static PifGpsNmea *s_pstGpsNmea = NULL;
static PifLed *s_pstLedL = NULL;


static void _evtGpsReceive(PifGps *pstOwner)
{
	PifDegMin stLatDegMin, stLonDegMin;
	PifDegMinSec stLatDegMinSec, stLonDegMinSec;

	pifLed_EachToggle(s_pstLedL, 1);

	pifGps_ConvertLatitude2DegMin(pstOwner, &stLatDegMin);
	pifGps_ConvertLongitude2DegMin(pstOwner, &stLonDegMin);

	pifGps_ConvertLatitude2DegMinSec(pstOwner, &stLatDegMinSec);
	pifGps_ConvertLongitude2DegMinSec(pstOwner, &stLonDegMinSec);

	if (!g_bPrintRawData) {
		pifLog_Printf(LT_INFO, "UTC Date Time: %4u/%2u/%2u %2u:%2u:%2u.%3u",
				2000 + pstOwner->_date_time.year, pstOwner->_date_time.month, pstOwner->_date_time.day,
				pstOwner->_date_time.hour, pstOwner->_date_time.minute, pstOwner->_date_time.second, pstOwner->_date_time.millisecond);
		pifLog_Printf(LT_INFO, "Longitude: %f` - %u`%f' - %u`%u'%f\"",
				pstOwner->_coord_deg[GPS_LON], stLonDegMin.degree, stLonDegMin.minute,
				stLonDegMinSec.degree, stLonDegMinSec.minute, stLonDegMinSec.second);
		pifLog_Printf(LT_INFO, "Latitude: %f` - %u`%f' - %u`%u'%f\"",
				pstOwner->_coord_deg[GPS_LAT], stLatDegMin.degree, stLatDegMin.minute,
				stLatDegMinSec.degree, stLatDegMinSec.minute, stLatDegMinSec.second);
		pifLog_Printf(LT_INFO, "NumSat: %u", pstOwner->_num_sat);
		pifLog_Printf(LT_INFO, "Altitude: %f m", pstOwner->_altitude);
		pifLog_Printf(LT_INFO, "Speed: %f knots %f m/s %f km/h", pstOwner->_speed_n, pifGps_ConvertKnots2MpS(pstOwner->_speed_n),
				pstOwner->_speed_k);
		pifLog_Printf(LT_INFO, "Ground Course: %f deg", pstOwner->_ground_course);
		pifLog_Printf(LT_INFO, "Fix: %u", pstOwner->_fix);
	}
	pifLog_Printf(LT_NONE, "\n");
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
	PifComm *pstCommLog;
	PifSensor *pstPushSwitch;

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
	pifGps_AttachEvent(&s_pstGpsNmea->_gps, _evtGpsReceive);
}
