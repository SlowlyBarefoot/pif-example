#include "appMain.h"
#include "exGpsSerialNmea.h"

#include "pifGpsNmea.h"
#include "pifLed.h"
#include "pifLog.h"
#include "pifSensorSwitch.h"


#define COMM_COUNT         		2
#define GPS_COUNT         		1
#define LED_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define SWITCH_COUNT            1
#define TASK_COUNT              4


PIF_stPulse *g_pstTimer1ms = NULL;
BOOL g_bPrintRawData = FALSE;

static PIF_stComm *s_pstCommGps = NULL;
static PIF_stGps *s_pstGps = NULL;
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
				2000 + pstOwner->_stDateTime.ucYear, pstOwner->_stDateTime.ucMonth, pstOwner->_stDateTime.ucDay,
				pstOwner->_stDateTime.ucHour, pstOwner->_stDateTime.ucMinute, pstOwner->_stDateTime.ucSecond, pstOwner->_stDateTime.usMilisecond);
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

static void _evtPushSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer)
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

    if (!pifComm_Init(COMM_COUNT)) return;
	if (!pifGps_Init(GPS_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);					// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_AddRatio(100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;				// 100%

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_AddPeriodMs(1, pifComm_Task, pstCommLog, TRUE)) return;				// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Add(PIF_ID_AUTO, 2, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;									// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

	pstPushSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, 0);
	if (!pstPushSwitch) return;
    if (!pifTask_AddPeriodMs(10, pifSensorSwitch_Task, pstPushSwitch, TRUE)) return;	// 10m
	pifSensor_AttachAction(pstPushSwitch, actPushSwitchAcquire);
	pifSensor_AttachEvtChange(pstPushSwitch, _evtPushSwitchChange, NULL);

	s_pstCommGps = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstCommGps) return;
    if (!pifTask_AddPeriodMs(1, pifComm_Task, s_pstCommGps, TRUE)) return;				// 1ms
	pifComm_AttachActReceiveData(s_pstCommGps, actGpsReceiveData);
	pifComm_AttachActSendData(s_pstCommGps, actGpsSendData);

	s_pstGps = pifGpsNmea_Add(PIF_ID_AUTO);
	if (!s_pstGps) return;
	if (!pifGpsNmea_SetProcessMessageId(s_pstGps, 2, NMEA_MESSAGE_ID_GGA, NMEA_MESSAGE_ID_VTG)) return;
	pifGpsNmea_SetEventMessageId(s_pstGps, NMEA_MESSAGE_ID_GGA);
	pifGpsNmea_AttachComm(s_pstGps, s_pstCommGps);
	pifGps_AttachEvent(s_pstGps, _evtGpsReceive);
}
