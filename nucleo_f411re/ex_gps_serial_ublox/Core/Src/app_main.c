#include "app_main.h"
#include "main.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "gps/pif_gps_ublox.h"


//#define NMEA
#define UBX


PifComm g_comm_log;
PifComm g_comm_gps;
PifTimerManager g_timer_1ms;
int g_print_data = 0;

static PifGpsUblox s_gps_ublox;
static PifLed s_led_l;
static uint32_t s_baudrate;
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

static void _evtGpsNmeaText(PifGpsNmeaTxt *pstTxt)
{
	const char *acType[4] = { "Error", "Warning", "Notice", "User" };

	pifLog_Printf(LT_NONE, "%s] %s\n", acType[pstTxt->type], pstTxt->text);
}

static BOOL _evtGpsNmeaReceive(PifGps *pstOwner, PifGpsNmeaMsgId msg_id)
{
	(void)pstOwner;

	return msg_id == PIF_GPS_NMEA_MSG_ID_GGA;
}

static uint16_t _taskNmeaSetup(PifTask *pstTask)
{
	uint32_t baudrates[] = { 115200, 57600, 38400, 19200, 9600 };
	uint8_t n;
	uint16_t delay = 100;
	static uint8_t step = 0, retry;

	pifLog_Printf(LT_INFO, "NMEA: Step=%xh", step);

	switch (step & 0xF0) {
	case 0x10:
		actGpsSetBaudrate(&g_comm_gps, baudrates[step - 0x10]);
		step += 0x10;
		retry = 2;
		delay = 200;
		break;

	case 0x20:
		if (pifGpsUblox_SetPubxConfig(&s_gps_ublox, 1, 0x07, 0x03, s_baudrate, TRUE, 0)) {
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d", GUCI_CFG, GUMI_CFG_PRT);
			retry--;
			if (!retry) {
				n = step - 0x20;
				n++;
				if (n < sizeof(baudrates) / sizeof(baudrates[0])) {
					step = (step - 0x10) + 1;
				}
				else {
					step = 0x30;
				}
				delay = 200;
			}
		}
		break;

	default:
		switch (step) {
		case 0:
			step = 0x10;
			delay = 5000;
			break;

		case 0x30:
			actGpsSetBaudrate(&g_comm_gps, s_baudrate);
			step++;
			delay = 1000;
			break;

		case 0x31:
			pstTask->pause = TRUE;
			s_booting = TRUE;
			break;
		}
		break;
	}
    return delay;
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

static uint16_t _taskUbloxSetup(PifTask *pstTask)
{
	uint32_t baudrates[] = { 115200, 57600, 38400, 19200, 9600 };
    const uint8_t kCfgMsg[][3] = {
			{ GUCI_NMEA_STD, GUMI_NMEA_VTG, 0x00 }, // Course over ground and Ground speed
			{ GUCI_NMEA_STD, GUMI_NMEA_GSV, 0x00 }, // GNSS Satellites in View
			{ GUCI_NMEA_STD, GUMI_NMEA_GLL, 0x00 }, // Latitude and longitude, with time of position fix and status
			{ GUCI_NMEA_STD, GUMI_NMEA_GGA, 0x00 }, // Global positioning system fix data
			{ GUCI_NMEA_STD, GUMI_NMEA_GSA, 0x00 }, // GNSS DOP and Active Satellites
			{ GUCI_NMEA_STD, GUMI_NMEA_RMC, 0x00 },	// Recommended Minimum data
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
	static uint8_t step = 0, retry;

	pifLog_Printf(LT_INFO, "UBX: Step=%xh", step);

	switch (step & 0xF0) {
	case 0x10:
		actGpsSetBaudrate(&g_comm_gps, baudrates[step - 0x10]);
		step += 0x10;
		retry = 2;
		break;

	case 0x20:
		if (pifGpsUblox_SetPubxConfig(&s_gps_ublox, 1, 0x07, 0x03, s_baudrate, TRUE, 0)) {
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d Retry=%d: Result=%d", GUCI_CFG, GUMI_CFG_PRT, retry, s_gps_ublox._request_state);
			retry--;
			if (!retry) {
				n = step - 0x20;
				n++;
				if (n < sizeof(baudrates) / sizeof(baudrates[0])) {
					step -= 0x10;
					step++;
				}
				else {
					step = 0x30;
				}
			}
		}
		break;

	case 0x40:
		n = step - 0x40;
		pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_MSG, sizeof(kCfgMsg[n]), (uint8_t*)kCfgMsg[n], TRUE, 200);
		if (s_gps_ublox._request_state == GURS_ACK) {
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d-%d: Result=%d", GUCI_CFG, GUMI_CFG_MSG, n, s_gps_ublox._request_state);
			n++;
			if (n < sizeof(kCfgMsg) / sizeof(kCfgMsg[0])) step++; else step = 0x50;
		}
		else {
			step = 0x10;
			delay = 500;
		}
		break;

	default:
		switch (step) {
		case 0:
			step = 0x10;
			delay = 5000;
			break;

		case 0x30:
			actGpsSetBaudrate(&g_comm_gps, s_baudrate);
			step = 0x40;
			delay = 500;
			break;

		case 0x50:
			p_rate = (uint16_t*)kCfgRate;
			*p_rate = 1000;	// 1000 = 1Hz
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_RATE, sizeof(kCfgRate), (uint8_t*)kCfgRate, TRUE, 200);
			if (s_gps_ublox._request_state == GURS_ACK) {
				pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_RATE, s_gps_ublox._request_state);
				step++;
			}
			else {
				step = 0x10;
				delay = 500;
			}
			break;

		case 0x51:
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_NAV5, sizeof(kCfgNav5), (uint8_t*)kCfgNav5, TRUE, 200);
			if (s_gps_ublox._request_state == GURS_ACK) {
				pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_NAV5, s_gps_ublox._request_state);
				step++;
			}
			else {
				step = 0x10;
				delay = 500;
			}
			break;

		case 0x52:
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_SBAS, sizeof(kCfgSbas[sbas]), (uint8_t*)kCfgSbas[sbas], TRUE, 200);
			if (s_gps_ublox._request_state == GURS_ACK) {
				pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_SBAS, s_gps_ublox._request_state);
				step++;
			}
			else {
				step = 0x10;
				delay = 500;
			}
			break;

		case 0x53:
			pstTask->pause = TRUE;
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

	pifLed_PartToggle(&s_led_l, 1 << 1);

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

void appSetup(uint32_t baurdate)
{
	s_baudrate = baurdate;

	pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;				// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE, "CommLog")) return;		// 1ms
	if (!pifComm_AllocRxBuffer(&g_comm_log, 64, 100)) return;							// 64bytes, 100%
	if (!pifComm_AllocTxBuffer(&g_comm_log, 128)) return;								// 128bytes
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;									// 500ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	if (!pifComm_Init(&g_comm_gps, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_gps, TM_PERIOD_MS, 1, TRUE, "CommGPS")) return;		// 1ms
	if (!pifComm_AllocRxBuffer(&g_comm_gps, 64, 100)) return;							// 256bytes, 100%
	if (!pifComm_AllocTxBuffer(&g_comm_gps, 32)) return;								// 32bytes
	g_comm_gps.act_start_transfer = actGpsStartTransfer;

	if (!pifGpsUblox_Init(&s_gps_ublox, PIF_ID_AUTO)) return;
	pifGpsUblox_AttachComm(&s_gps_ublox, &g_comm_gps);
	s_gps_ublox._gps.evt_receive = _evtGpsReceive;
#ifdef NMEA
	s_gps_ublox._gps.evt_nmea_receive = _evtGpsNmeaReceive;
	if (!pifGps_SetEventNmeaText(&s_gps_ublox._gps, _evtGpsNmeaText)) return;
	if (!pifTaskManager_Add(TM_CHANGE_MS, 100, _taskNmeaSetup, NULL, TRUE)) return;		// 100ms
#endif
#ifdef UBX
	s_gps_ublox.evt_ubx_receive = _evtGpsUbxReceive;
	if (!pifTaskManager_Add(TM_CHANGE_MS, 100, _taskUbloxSetup, NULL, TRUE)) return;	// 100ms
#endif

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
