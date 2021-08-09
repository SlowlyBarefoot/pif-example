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
#define TASK_COUNT              3


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
	PIF_stSensor *pstPushSwitch = NULL;

	pif_Init(NULL);

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    s_pstLedL = pifLed_Add(PIF_ID_AUTO, 2, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;						// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;

	pstPushSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, 0);
	if (!pstPushSwitch) return;
	pifSensor_AttachAction(pstPushSwitch, actPushSwitchAcquire);
	pifSensor_AttachEvtChange(pstPushSwitch, _evtPushSwitchChange, NULL);

    if (!pifComm_Init(COMM_COUNT)) return;

	s_pstCommGps = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstCommGps) return;
	pifComm_AttachActReceiveData(s_pstCommGps, actGpsReceiveData);
	pifComm_AttachActSendData(s_pstCommGps, actGpsSendData);

	if (!pifGps_Init(GPS_COUNT)) return;
	s_pstGps = pifGpsNmea_Add(PIF_ID_AUTO);
	if (!s_pstGps) return;
	if (!pifGpsNmea_SetProcessMessageId(s_pstGps, 2, NMEA_MESSAGE_ID_GGA, NMEA_MESSAGE_ID_VTG)) return;
	pifGpsNmea_SetEventMessageId(s_pstGps, NMEA_MESSAGE_ID_GGA);
	pifGpsNmea_AttachComm(s_pstGps, s_pstCommGps);
	pifGps_AttachEvent(s_pstGps, _evtGpsReceive);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddPeriodMs(10, pifSensorSwitch_taskAll, NULL)) return;	// 10m
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;				// 1ms
}
