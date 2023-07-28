#include "app_main.h"

#include "gps/pif_gps_nmea.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_gps;

static int s_print_data = 1;
static PifGpsNmea s_gps_nmea;

static int _cmdPrintData(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "print", _cmdPrintData, "Print Data", NULL },

	{ NULL, NULL, NULL, NULL }
};


static void _evtGpsNmeaFrame(char* p_frame)
{
	pifLog_Print(LT_NONE, p_frame);
}

static void _evtGpsNmeaText(PifGpsNmeaTxt *pstTxt)
{
	const char *acType[4] = { "Error", "Warning", "Notice", "User" };

	pifLog_Printf(LT_NONE, "%s] %s\n", acType[pstTxt->type], pstTxt->text);
}

static int _cmdPrintData(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Print Data: %d\n", s_print_data);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		switch (argv[0][0]) {
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

static BOOL _evtGpsNmeaReceive(PifGps *pstOwner, PifGpsNmeaMsgId msg_id)
{
	(void)pstOwner;

	return msg_id == PIF_GPS_NMEA_MSG_ID_GGA;
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

BOOL appSetup()
{
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return FALSE;

	if (!pifGpsNmea_Init(&s_gps_nmea, PIF_ID_AUTO)) return FALSE;
	pifGpsNmea_AttachUart(&s_gps_nmea, &g_uart_gps);
	if (!pifGps_SetEventNmeaText(&s_gps_nmea._gps, _evtGpsNmeaText)) return FALSE;
	s_gps_nmea._gps.evt_nmea_receive = _evtGpsNmeaReceive;
	s_gps_nmea._gps.evt_receive = _evtGpsReceive;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;		// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
