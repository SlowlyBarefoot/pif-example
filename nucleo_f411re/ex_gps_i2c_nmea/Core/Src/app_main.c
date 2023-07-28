#include "linker.h"

#include "gps/pif_gps_nmea.h"


PifI2cPort g_i2c_port;
PifLed g_led_l;
PifTimerManager g_timer_1ms;

int g_print_data = 0;

static PifGpsNmea s_gps_nmea;

static BOOL s_booting = FALSE;

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

static BOOL _evtGpsNmeaReceive(PifGps *p_owner, PifGpsNmeaMsgId msg_id)
{
	(void)p_owner;

	return msg_id == PIF_GPS_NMEA_MSG_ID_GGA;
}

static void _evtGpsReceive(PifGps *p_owner)
{
	PifDegMin stLatDegMin, stLonDegMin;
	PifDegMinSec stLatDegMinSec, stLonDegMinSec;

	pifLed_PartToggle(&g_led_l, 1 << 1);

	if (!s_booting) return;

	pifGps_ConvertLatitude2DegMin(p_owner, &stLatDegMin);
	pifGps_ConvertLongitude2DegMin(p_owner, &stLonDegMin);

	pifGps_ConvertLatitude2DegMinSec(p_owner, &stLatDegMinSec);
	pifGps_ConvertLongitude2DegMinSec(p_owner, &stLonDegMinSec);

	pifLog_Printf(LT_INFO, "UTC Date Time: %4u/%2u/%2u %2u:%2u:%2u.%3u",
			2000 + p_owner->_utc.year, p_owner->_utc.month, p_owner->_utc.day,
			p_owner->_utc.hour, p_owner->_utc.minute, p_owner->_utc.second, p_owner->_utc.millisecond);
	pifLog_Printf(LT_INFO, "Longitude: %f` - %u`%f' - %u`%u'%f\"",
			p_owner->_coord_deg[PIF_GPS_LON], stLonDegMin.degree, stLonDegMin.minute,
			stLonDegMinSec.degree, stLonDegMinSec.minute, stLonDegMinSec.second);
	pifLog_Printf(LT_INFO, "Latitude: %f` - %u`%f' - %u`%u'%f\"",
			p_owner->_coord_deg[PIF_GPS_LAT], stLatDegMin.degree, stLatDegMin.minute,
			stLatDegMinSec.degree, stLatDegMinSec.minute, stLatDegMinSec.second);
	pifLog_Printf(LT_INFO, "NumSat: %u", p_owner->_num_sat);
	pifLog_Printf(LT_INFO, "Altitude: %f m", p_owner->_altitude);
	pifLog_Printf(LT_INFO, "Speed: %f cm/s", p_owner->_ground_speed);
	pifLog_Printf(LT_INFO, "Ground Course: %f deg", p_owner->_ground_course);
	pifLog_Printf(LT_INFO, "Fix: %u", p_owner->_fix);
	pifLog_Printf(LT_NONE, "\n");
}

static int _cmdPrintData(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Print Data: %d\n", g_print_data);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		switch (argv[0][0]) {
		case '0':
		case 'F':
		case 'f':
			g_print_data = 0;
			s_gps_nmea._gps.evt_frame = NULL;
			break;

		case '1':
		case 'T':
		case 't':
			g_print_data = 1;
			s_gps_nmea._gps.evt_frame = NULL;
			break;

		case '2':
		case 'R':
		case 'r':
			g_print_data = 2;
			s_gps_nmea._gps.evt_frame = _evtGpsNmeaFrame;
			break;

		default:
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

BOOL appSetup()
{
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return FALSE;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;											// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

	if (!pifGpsNmea_Init(&s_gps_nmea, PIF_ID_AUTO)) return FALSE;
	if (!pifGpsNmea_AttachI2c(&s_gps_nmea, &g_i2c_port, 0x42, 250, TRUE, NULL)) return FALSE;		// 0x42 : Ublox I2c addrress, 500ms
	s_gps_nmea._gps.evt_receive = _evtGpsReceive;
	s_gps_nmea._gps.evt_nmea_receive = _evtGpsNmeaReceive;
	if (!pifGps_SetEventNmeaText(&s_gps_nmea._gps, _evtGpsNmeaText)) return FALSE;

	s_booting = TRUE;
	return TRUE;
}
