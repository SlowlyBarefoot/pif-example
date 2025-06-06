#include "appMain.h"

#include "gps/pif_gps_nmea.h"


PifLed g_led_l;
PifSensorSwitch g_push_switch;
PifTimerManager g_timer_1ms;
PifUart g_uart_gps;

static int s_print_data = 1;
static PifGpsNmea s_gps_nmea;


static void _evtGpsNmeaFrame(char* p_frame)
{
	pifLog_Print(LT_NONE, p_frame);
}

static BOOL _evtGpsNmeaReceive(PifGps *pstOwner, PifGpsNmeaMsgId msg_id)
{
	(void)pstOwner;

	return msg_id == PIF_GPS_NMEA_MSG_ID_GLL;
}

static void _evtGpsReceive(PifGps *pstOwner)
{
	PifDegMin stLatDegMin, stLonDegMin;
	PifDegMinSec stLatDegMinSec, stLonDegMinSec;

	pifLed_PartToggle(&g_led_l, 1 << 1);

	pifGps_ConvertLatitude2DegMin(pstOwner, &stLatDegMin);
	pifGps_ConvertLongitude2DegMin(pstOwner, &stLonDegMin);

	pifGps_ConvertLatitude2DegMinSec(pstOwner, &stLatDegMinSec);
	pifGps_ConvertLongitude2DegMinSec(pstOwner, &stLonDegMinSec);

	if (s_print_data == 1) {
		pifLog_Printf(LT_INFO, "UTC Date Time: %4u/%2u/%2u %2u:%2u:%2u.%3u",
				2000 + pstOwner->_utc.year, pstOwner->_utc.month, pstOwner->_utc.day,
				pstOwner->_utc.hour, pstOwner->_utc.minute, pstOwner->_utc.second, pstOwner->_utc.millisecond);
		pifLog_Printf(LT_INFO, "Longitude: %f` - %u`%f' - %u`%u'%f\"",
				pstOwner->_coord_deg[PIF_GPS_LON], stLonDegMin.degree, stLonDegMin.minute,
				stLonDegMinSec.degree, stLonDegMinSec.minute, stLonDegMinSec.second);
		pifLog_Printf(LT_INFO, "Latitude: %f` - %u`%f' - %u`%u'%f\"",
				pstOwner->_coord_deg[PIF_GPS_LAT], stLatDegMin.degree, stLatDegMin.minute,
				stLatDegMinSec.degree, stLatDegMinSec.minute, stLatDegMinSec.second);
		pifLog_Printf(LT_INFO, "NumSat: %u", pstOwner->_num_sat);
		pifLog_Printf(LT_INFO, "Altitude: %f m", pstOwner->_altitude);
		pifLog_Printf(LT_INFO, "Speed: %f cm/s", pstOwner->_ground_speed);
		pifLog_Printf(LT_INFO, "Ground Course: %f deg", pstOwner->_ground_course);
		pifLog_Printf(LT_INFO, "Fix: %u", pstOwner->_fix);
	}
	if (s_print_data) {
		pifLog_Printf(LT_NONE, "\n");
	}
}

static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	if (state) {
		s_print_data = (s_print_data + 1) % 3;
		if (s_print_data == 2) {
			s_gps_nmea._gps.evt_frame = _evtGpsNmeaFrame;
		}
		else {
			s_gps_nmea._gps.evt_frame = NULL;
		}
	}
}

BOOL appSetup()
{
    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch, PIF_ID_AUTO, TM_PERIOD, 10000, TRUE)) return FALSE;	// 10m
    pifSensor_AttachEvtChange(&g_push_switch.parent, _evtPushSwitchChange, NULL);

	if (!pifGpsNmea_Init(&s_gps_nmea, PIF_ID_AUTO)) return FALSE;
	pifGpsNmea_AttachUart(&s_gps_nmea, &g_uart_gps);
	s_gps_nmea._gps.evt_nmea_receive = _evtGpsNmeaReceive;
	s_gps_nmea._gps.evt_receive = _evtGpsReceive;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;														// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
