#include "appMain.h"
#include "exGpsSerialNmea.h"

#include "pif_gps_nmea.h"
#include "pif_led.h"
#include "pif_log.h"


PifTimerManager g_timer_1ms;

static int s_print_data = 1;
static PifComm s_comm_gps;
static PifGpsNmea s_gps_nmea;
static PifLed s_led_l;

static int _cmdPrintData(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "print", _cmdPrintData, "Print Data" },

	{ NULL, NULL, NULL }
};


static void _evtGpsNmeaFrame(char* p_frame)
{
	pifLog_Print(LT_NONE, p_frame);
}

static void _evtGpsNmeaText(PifGpsNmeaTxt *pstTxt)
{
	const char *acType[4] = { "Error", "Warning", "Notice", "User" };

	pifLog_Printf(LT_INFO, "Text: Total=%u Num=%u Type=%s:%s", pstTxt->total, pstTxt->num, acType[pstTxt->type], pstTxt->text);
}

static int _cmdPrintData(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "  Print Data: %d\n", s_print_data);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		switch (argv[1][0]) {
		case '0':
		case 'F':
		case 'f':
			s_print_data = 0;
			s_gps_nmea._gps.evt_frame = NULL;
			break;

		case '1':
		case 'T':
		case 't':
			s_print_data = 1;
			s_gps_nmea._gps.evt_frame = NULL;
			break;

		case '2':
		case 'R':
		case 'r':
			s_print_data = 2;
			s_gps_nmea._gps.evt_frame = _evtGpsNmeaFrame;
			break;

		default:
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
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
				2000 + pstOwner->_date_time.year, pstOwner->_date_time.month, pstOwner->_date_time.day,
				pstOwner->_date_time.hour, pstOwner->_date_time.minute, pstOwner->_date_time.second, pstOwner->_date_time.millisecond);
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

void appSetup()
{
	static PifComm s_comm_log;

	pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	s_comm_log.act_receive_data = actLogReceiveData;
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;						// 500ms

	if (!pifComm_Init(&s_comm_gps, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_gps, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
    s_comm_gps.act_receive_data = actGpsReceiveData;
    s_comm_gps.act_send_data = actGpsSendData;

	if (!pifGpsNmea_Init(&s_gps_nmea, PIF_ID_AUTO)) return;
	pifGpsNmea_AttachComm(&s_gps_nmea, &s_comm_gps);
	pifGps_SetEventNmeaText(&s_gps_nmea._gps, _evtGpsNmeaText);
	s_gps_nmea._gps.evt_nmea_msg_id = PIF_GPS_NMEA_MSG_ID_GGA;
	s_gps_nmea._gps.evt_receive = _evtGpsReceive;

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
