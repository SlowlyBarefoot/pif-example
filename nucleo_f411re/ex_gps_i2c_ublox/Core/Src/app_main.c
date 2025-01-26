#include "linker.h"

#include "gps/pif_gps_ublox.h"


//#define NMEA
#define UBX


PifI2cPort g_i2c_port;
PifLed g_led_l;
PifTimerManager g_timer_1ms;

int g_print_data = 0;

static PifGpsUblox s_gps_ublox;

static BOOL s_booting = FALSE;

static int _cmdPrintData(int argc, char *argv[]);
static int _cmdPollRequest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "print", _cmdPrintData, "Print Data", NULL },
	{ "req", _cmdPollRequest, "Poll Request", NULL },

	{ NULL, NULL, NULL, NULL }
};


#ifdef NMEA

static void _evtGpsNmeaFrame(char* p_frame)
{
	pifLog_Print(LT_NONE, p_frame);
}

static void _evtGpsNmeaText(PifGpsNmeaTxt *p_txt)
{
	const char *acType[4] = { "Error", "Warning", "Notice", "User" };

	pifLog_Printf(LT_NONE, "%s] %s\n", acType[p_txt->type], p_txt->text);
}

static BOOL _evtGpsNmeaReceive(PifGps *p_owner, PifGpsNmeaMsgId msg_id)
{
	(void)p_owner;

	return msg_id == PIF_GPS_NMEA_MSG_ID_GGA;
}

#endif

#ifdef UBX

static BOOL _evtGpsUbxReceive(PifGpsUblox* p_owner, PifGpsUbxPacket* p_packet)
{
    switch (p_packet->class_id) {
    case GUCI_ACK:
    	pifLog_Printf(LT_INFO, "Ubx: cid=%u mid=%u, cid=%u mid=%u", p_packet->class_id, p_packet->msg_id, p_packet->payload.bytes[0], p_packet->payload.bytes[1]);
    	break;

    case GUCI_NAV:
    	switch (p_packet->msg_id) {
    	case GUMI_NAV_SVINFO:
    		if (g_print_data) {
				pifLog_Printf(LT_INFO, "SvInfo: ch=%u rate=%lu:%lu", s_gps_ublox._num_ch, s_gps_ublox._svinfo_rate[0], s_gps_ublox._svinfo_rate[1]);
				for (int i = 0; i < s_gps_ublox._num_ch; i++) {
					pifLog_Printf(LT_INFO, "   ch=%u rate=%lu:%lu", s_gps_ublox._svinfo_chn[i], s_gps_ublox._svinfo_svid[i], s_gps_ublox._svinfo_quality[i], s_gps_ublox._svinfo_cno[i]);
				}
    		}
			break;
    	case GUMI_NAV_POSLLH:
		    return TRUE;
		}
		break;
    }
	return FALSE;
}

static uint16_t _taskUbloxSetup(PifTask *p_task)
{
    const uint8_t kCfgMsgNmea[][3] = {
			{ GUCI_NMEA_STD, GUMI_NMEA_VTG, 0x00 }, // Course over ground and Ground speed
			{ GUCI_NMEA_STD, GUMI_NMEA_GSV, 0x00 }, // GNSS Satellites in View
			{ GUCI_NMEA_STD, GUMI_NMEA_GLL, 0x00 }, // Latitude and longitude, with time of position fix and status
			{ GUCI_NMEA_STD, GUMI_NMEA_GGA, 0x00 }, // Global positioning system fix data
			{ GUCI_NMEA_STD, GUMI_NMEA_GSA, 0x00 }, // GNSS DOP and Active Satellites
			{ GUCI_NMEA_STD, GUMI_NMEA_RMC, 0x00 },	// Recommended Minimum data
    };
    const uint8_t kCfgMsgNav[][3] = {
		    { GUCI_NAV, GUMI_NAV_POSLLH, 0x01 },	// set POSLLH MSG rate
		    { GUCI_NAV, GUMI_NAV_SOL, 0x01 },   	// set SOL MSG rate
		    { GUCI_NAV, GUMI_NAV_SVINFO, 0x05 },   	// set SVINFO MSG rate
		    { GUCI_NAV, GUMI_NAV_TIMEUTC, 0x01 },   // set TIMEUTC MSG rate
		    { GUCI_NAV, GUMI_NAV_VELNED, 0x01 }    	// set VELNED MSG rate
    };
    uint8_t kCfgRate[] = {
    		0xE8, 0x03,				// messRate 1Hz
			0x01, 0x00, 			// navRate
			0x01, 0x00 				// timeRef
    };
    const uint8_t kCfgNav5[] = {
    		0xFF, 0xFF,						// mask
			0x06, 							// dynModel = airborne with < 1g acceleration
			0x03, 							// fixMode = auto 2D/3D
			0x00, 0x00,	0x00, 0x00, 		// fixedAlt
			0x10, 0x27, 0x00, 0x00,			// fixedAltVar
			0x05, 							// minElev
			0x00,							// drLimit
			0xFA, 0x00,  					// pDop
			0xFA, 0x00,						// tDop
			0x64, 0x00, 					// pAcc
			0x2C, 0x01,						// tAcc
			0x00,							// staticHoldThresh
			0x00,							// dgnssTimeout
			0x00,  							// cnoThreshNumSVs
			0x00, 							// cnoThresh
			0x00, 0x00,	 					// reserved1
			0x00, 0x00,						// staticHoldMaxDist
			0x00, 							// utcStandard
			0x00, 0x00, 0x00, 0x00, 0x00	// reserved2
    };
    const uint8_t kCfgSbas[][8] = {
			{ 0x03, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 },   	// Auto
	        { 0x03, 0x07, 0x03, 0x00, 0x4A, 0x00, 0x01, 0x40 },   	// EGNOS
	        { 0x03, 0x07, 0x03, 0x00, 0x00, 0xA8, 0x04, 0x00 },   	// WAAS
	        { 0x03, 0x07, 0x03, 0x00, 0x00, 0x42, 0x0A, 0x00 },   	// MSAS + KASS
	        { 0x03, 0x07, 0x03, 0x00, 0x80, 0x11, 0x00, 0x00 },   	// GAGAN
			{ 0x02, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 }		// Disabled
    };
	uint16_t* p_rate;
	uint8_t sbas = 0;		// 0 = Auto
	uint8_t n;
	uint16_t delay = 100;
	static uint8_t step = 0;

	pifLog_Printf(LT_INFO, "UBX: Step=%x", step);

	switch (step & 0xF0) {
	case 0x10:
		n = step - 0x10;
		pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_MSG, sizeof(kCfgMsgNmea[n]), (uint8_t*)kCfgMsgNmea[n], TRUE, 400);
		if (s_gps_ublox._request_state == GURS_ACK) {
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d-%d: Result=%d", GUCI_CFG, GUMI_CFG_MSG, n, s_gps_ublox._request_state);
			n++;
			if (n < sizeof(kCfgMsgNmea) / sizeof(kCfgMsgNmea[0])) step++;
		else {
				pifTask_ChangePeriod(s_gps_ublox._p_task, 50);	// 50ms
				step = 0x20;
			}
			delay = 400;
		}
		else delay = 500;
		break;

	case 0x20:
		n = step - 0x20;
		pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_MSG, sizeof(kCfgMsgNav[n]), (uint8_t*)kCfgMsgNav[n], TRUE, 100);
		if (s_gps_ublox._request_state == GURS_ACK) {
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d-%d: Result=%d", GUCI_CFG, GUMI_CFG_MSG, n, s_gps_ublox._request_state);
			n++;
			if (n < sizeof(kCfgMsgNav) / sizeof(kCfgMsgNav[0])) step++; else step = 0x30;
		}
		else delay = 500;
		break;

	default:
		switch (step) {
		case 0:
			step = 0x10;
			delay = 5000;
			break;

		case 0x30:
			p_rate = (uint16_t*)kCfgRate;
			*p_rate = 1000;	// 1000 = 1Hz
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_RATE, sizeof(kCfgRate), (uint8_t*)kCfgRate, TRUE, 100);
			if (s_gps_ublox._request_state == GURS_ACK) {
				pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_RATE, s_gps_ublox._request_state);
				step++;
			}
			else delay = 500;
			break;

		case 0x31:
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_NAV5, sizeof(kCfgNav5), (uint8_t*)kCfgNav5, TRUE, 100);
			if (s_gps_ublox._request_state == GURS_ACK) {
				pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_NAV5, s_gps_ublox._request_state);
				step++;
			}
			else delay = 500;
			break;

		case 0x32:
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_SBAS, sizeof(kCfgSbas[sbas]), (uint8_t*)kCfgSbas[sbas], TRUE, 100);
			if (s_gps_ublox._request_state == GURS_ACK) {
				pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_SBAS, s_gps_ublox._request_state);
				step++;
			}
			else delay = 500;
			break;

		case 0x33:
			pifTask_ChangePeriod(s_gps_ublox._p_task, 500);	// 500ms
			p_task->pause = TRUE;
			s_booting = TRUE;
			break;
		}
		break;
	}
    return delay;
}

#endif

static void _evtGpsReceive(PifGps *p_owner)
{
	PifDegMin lat_deg_min, lon_deg_min;
	PifDegMinSec lat_deg_min_sec, lon_deg_min_sec;

	pifLed_PartToggle(&g_led_l, 1 << 1);

	if (!s_booting) return;

	pifGps_ConvertLatitude2DegMin(p_owner, &lat_deg_min);
	pifGps_ConvertLongitude2DegMin(p_owner, &lon_deg_min);

	pifGps_ConvertLatitude2DegMinSec(p_owner, &lat_deg_min_sec);
	pifGps_ConvertLongitude2DegMinSec(p_owner, &lon_deg_min_sec);

	pifLog_Printf(LT_INFO, "UTC Date Time: %4u/%2u/%2u %2u:%2u:%2u.%3u",
			2000 + p_owner->_utc.year, p_owner->_utc.month, p_owner->_utc.day,
			p_owner->_utc.hour, p_owner->_utc.minute, p_owner->_utc.second, p_owner->_utc.millisecond);
	pifLog_Printf(LT_INFO, "Longitude: %f` - %u`%f' - %u`%u'%f\"",
			p_owner->_coord_deg[PIF_GPS_LON], lon_deg_min.degree, lon_deg_min.minute,
			lon_deg_min_sec.degree, lon_deg_min_sec.minute, lon_deg_min_sec.second);
	pifLog_Printf(LT_INFO, "Latitude: %f` - %u`%f' - %u`%u'%f\"",
			p_owner->_coord_deg[PIF_GPS_LAT], lat_deg_min.degree, lat_deg_min.minute,
			lat_deg_min_sec.degree, lat_deg_min_sec.minute, lat_deg_min_sec.second);
	pifLog_Printf(LT_INFO, "NumSat: %u", p_owner->_num_sat);
	pifLog_Printf(LT_INFO, "Altitude: %f m", p_owner->_altitude);
	pifLog_Printf(LT_INFO, "Speed: %f cm/s", p_owner->_ground_speed);
	pifLog_Printf(LT_INFO, "Ground Course: %f deg", p_owner->_ground_course);
	pifLog_Printf(LT_INFO, "Fix: %u", p_owner->_fix);
	pifLog_Printf(LT_INFO, "Acc: Hor %lu mm Ver %lu mm", p_owner->_horizontal_acc, p_owner->_vertical_acc);
	pifLog_Printf(LT_INFO, "Update: %lu ms", p_owner->_update_rate[1] - p_owner->_update_rate[0]);
/*		pifLog_Printf(LT_INFO, "SvInfo: ch=%u rate=%lu:%lu", s_gps_ublox._num_ch, s_gps_ublox._svinfo_rate[0], s_gps_ublox._svinfo_rate[1]);
	for (int i = 0; i < s_gps_ublox._num_ch; i++) {
		pifLog_Printf(LT_INFO, "   ch=%u rate=%lu:%lu", s_gps_ublox._svinfo_chn[i], s_gps_ublox._svinfo_svid[i], s_gps_ublox._svinfo_quality[i], s_gps_ublox._svinfo_cno[i]);
	} */
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
#ifdef NMEA
			s_gps_ublox._gps.evt_frame = NULL;
#endif
			break;

		case '1':
		case 'T':
		case 't':
			g_print_data = 1;
#ifdef NMEA
			s_gps_ublox._gps.evt_frame = NULL;
#endif
			break;

		case '2':
		case 'R':
		case 'r':
			g_print_data = 2;
#ifdef NMEA
			s_gps_ublox._gps.evt_frame = _evtGpsNmeaFrame;
#endif
			break;

		default:
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _cmdPollRequest(int argc, char *argv[])
{
	if (argc > 1) {
		if (strcmp(argv[0], "GBQ") == 0) {
			if (!pifGpsUblox_PollRequestGBQ(&s_gps_ublox, argv[1], FALSE, 0)) {
				pifLog_Printf(LT_ERROR, "Error: %u", pif_error);
			}
		}
		else if (strcmp(argv[0], "GLQ") == 0) {
			if (!pifGpsUblox_PollRequestGLQ(&s_gps_ublox, argv[1], FALSE, 0)) {
				pifLog_Printf(LT_ERROR, "Error: %u", pif_error);
			}
		}
		else if (strcmp(argv[0], "GNQ") == 0) {
			if (!pifGpsUblox_PollRequestGNQ(&s_gps_ublox, argv[1], FALSE, 0)) {
				pifLog_Printf(LT_ERROR, "Error: %u", pif_error);
			}
		}
		else if (strcmp(argv[0], "GPQ") == 0) {
			if (!pifGpsUblox_PollRequestGPQ(&s_gps_ublox, argv[1], FALSE, 0)) {
				pifLog_Printf(LT_ERROR, "Error: %u", pif_error);
			}
		}
		else {
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

	if (!pifGpsUblox_Init(&s_gps_ublox, PIF_ID_AUTO)) return FALSE;
	if (!pifGpsUblox_AttachI2c(&s_gps_ublox, &g_i2c_port, 0x42, 30, 500, TRUE, NULL)) return FALSE;		// 0x42 : Ublox I2c addrress, 500ms
	s_gps_ublox._gps.evt_receive = _evtGpsReceive;
#ifdef NMEA
	s_gps_ublox._gps.evt_nmea_receive = _evtGpsNmeaReceive;
	if (!pifGps_SetEventNmeaText(&s_gps_ublox._gps, _evtGpsNmeaText)) return FALSE;
	s_booting = TRUE;
#endif
#ifdef UBX
	s_gps_ublox.evt_ubx_receive = _evtGpsUbxReceive;
	if (!pifTaskManager_Add(TM_CHANGE_MS, 100, _taskUbloxSetup, NULL, TRUE)) return FALSE;			// 100ms
#endif
	return TRUE;
}
