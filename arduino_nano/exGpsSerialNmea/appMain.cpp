#include "appMain.h"
#include "exGpsSerialNmea.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "gps/pif_gps_nmea.h"
#include "sensor/pif_sensor_switch.h"


PifTimerManager g_timer_1ms;

static int s_print_data = 1;
static PifComm s_comm_gps;
static PifGpsNmea s_gps_nmea;
static PifLed s_led_l;


static void _evtGpsNmeaFrame(char* p_frame)
{
	pifLog_Print(LT_NONE, p_frame);
}

static void _evtGpsReceive(PifGps *pstOwner)
{
	PifDegMin stLatDegMin, stLonDegMin;
	PifDegMinSec stLatDegMinSec, stLonDegMinSec;

	pifLed_PartToggle(&s_led_l, 1 << 1);

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

void appSetup()
{
	static PifComm s_comm_log;
	static PifSensorSwitch s_push_switch;

	pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;					// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;					// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;										// 500ms

	if (!pifSensorSwitch_Init(&s_push_switch, PIF_ID_AUTO, 0, actPushSwitchAcquire, NULL)) return;
    if (!pifSensorSwitch_AttachTaskAcquire(&s_push_switch, TM_PERIOD_MS, 10, TRUE)) return;	// 10m
	pifSensor_AttachEvtChange(&s_push_switch.parent, _evtPushSwitchChange);

	if (!pifComm_Init(&s_comm_gps, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_gps, TM_PERIOD_MS, 1, TRUE)) return;					// 1ms
    s_comm_gps.act_receive_data = actGpsReceiveData;
    s_comm_gps.act_send_data = actGpsSendData;

	if (!pifGpsNmea_Init(&s_gps_nmea, PIF_ID_AUTO)) return;
	pifGpsNmea_AttachComm(&s_gps_nmea, &s_comm_gps);
	s_gps_nmea._gps.evt_nmea_msg_id = PIF_GPS_NMEA_MSG_ID_GGA;
	s_gps_nmea._gps.evt_receive = _evtGpsReceive;

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
