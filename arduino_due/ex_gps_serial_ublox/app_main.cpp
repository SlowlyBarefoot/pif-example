#include "app_main.h"
#include "ex_gps_serial_ublox.h"

#include "pif_gps_ublox.h"
#include "pif_led.h"
#include "pif_log.h"


//#define NMEA
#define UBX


PifTimerManager g_timer_1ms;
int g_print_data = 1;

static PifComm s_comm_gps;
static PifGpsUblox s_gps_ublox;
static PifLed s_led_l;

static int _cmdPrintData(int argc, char *argv[]);
static int _cmdPollRequest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "print", _cmdPrintData, "Print Data" },
	{ "req", _cmdPollRequest, "Poll Request" },

	{ NULL, NULL, NULL }
};


static int _cmdPrintData(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_NONE, "  Print Data: %d\n", g_print_data);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		switch (argv[1][0]) {
		case '0':
		case 'F':
		case 'f':
			g_print_data = 0;
			break;

		case '1':
		case 'T':
		case 't':
			g_print_data = 1;
			break;

		case '2':
		case 'R':
		case 'r':
			g_print_data = 2;
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
	if (argc > 2) {
		if (strcmp(argv[1], "GBQ") == 0) {
			if (!pifGpsUblox_PollRequestGBQ(&s_gps_ublox, argv[2], FALSE)) {
				pifLog_Printf(LT_ERROR, "Error: %u", pif_error);
			}
		}
		else if (strcmp(argv[1], "GLQ") == 0) {
			if (!pifGpsUblox_PollRequestGLQ(&s_gps_ublox, argv[2], FALSE)) {
				pifLog_Printf(LT_ERROR, "Error: %u", pif_error);
			}
		}
		else if (strcmp(argv[1], "GNQ") == 0) {
			if (!pifGpsUblox_PollRequestGNQ(&s_gps_ublox, argv[2], FALSE)) {
				pifLog_Printf(LT_ERROR, "Error: %u", pif_error);
			}
		}
		else if (strcmp(argv[1], "GPQ") == 0) {
			if (!pifGpsUblox_PollRequestGPQ(&s_gps_ublox, argv[2], FALSE)) {
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

static void _evtGpsReceive(PifGps *p_owner)
{
	PifDegMin lat_deg_min, lon_deg_min;
	PifDegMinSec lat_deg_min_sec, lon_deg_min_sec;

	pifLed_PartToggle(&s_led_l, 1 << 1);

	pifGps_ConvertLatitude2DegMin(p_owner, &lat_deg_min);
	pifGps_ConvertLongitude2DegMin(p_owner, &lon_deg_min);

	pifGps_ConvertLatitude2DegMinSec(p_owner, &lat_deg_min_sec);
	pifGps_ConvertLongitude2DegMinSec(p_owner, &lon_deg_min_sec);

	if (g_print_data == 1) {
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
		pifLog_Printf(LT_INFO, "Update: %lu ms, %lu ms", p_owner->_update_rate[0], p_owner->_update_rate[1]);
	}
	if (g_print_data) {
		pifLog_Printf(LT_NONE, "\n");
	}
}

void _evtGpsUbxReceive(PifGpsUbxPacket* p_packet)
{
	pifLog_Printf(LT_INFO, "UBX: CID=%u MID=%u Len=%u", p_packet->class_id, p_packet->msg_id, p_packet->length);
}

void appSetup()
{
	static PifComm s_comm_log;
#ifdef UBX
    const uint8_t kCfgPrt[] = {
    		0x01, 					// portID UART
			0x00, 					// reserved1
			0x00, 0x00, 			// txReady
			0xC0, 0x08, 0x00, 0x00,	// mode 8 bits, No parity, 1 Stop Bits
			0x00, 0xC2, 0x01, 0x00, // bardRate 115200 bits/s
			0x07, 0x00,				// inProtoMask UBX+NMEA+RCTM2
			0x03, 0x00,				// outProtoMask UBX+NMEA
			0x00, 0x00,				// flags
			0x00, 0x00				// reserved2
    };
    const uint8_t kCfgMsg[][3] = {
			{ GUCI_NMEA_STD, GUMI_NMEA_VTG, 0x00 }, // Course over ground and Ground speed
			{ GUCI_NMEA_STD, GUMI_NMEA_GSV, 0x00 }, // GNSS Satellites in View
			{ GUCI_NMEA_STD, GUMI_NMEA_GLL, 0x00 }, // Latitude and longitude, with time of position fix and status
			{ GUCI_NMEA_STD, GUMI_NMEA_GGA, 0x00 }, // Global positioning system fix data
			{ GUCI_NMEA_STD, GUMI_NMEA_GSA, 0x00 }, // GNSS DOP and Active Satellites
			{ GUCI_NMEA_STD, GUMI_NMEA_RMC, 0x00 },	// Recommended Minimum data
		    { GUCI_NAV, GUMI_NAV_POSLLH, 0x01 },	// set POSLLH MSG rate
		    { GUCI_NAV, GUMI_NAV_SOL, 0x01 },   	// set SOL MSG rate
		    { GUCI_NAV, GUMI_NAV_TIMEUTC, 0x01 },   // set TIMEUTC MSG rate
		    { GUCI_NAV, GUMI_NAV_VELNED, 0x01 }    	// set VELNED MSG rate
    };
    const uint8_t kCfgRate[] = {
    		0xC8, 0x00,				// messRate 5Hz
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
			{ 0x03, 0x07, 0x03, 0x00, 0x51, 0x08, 0x00, 0x00 },   	// EGNOS
			{ 0x03, 0x07, 0x03, 0x00, 0x04, 0xE0, 0x04, 0x00 },   	// WAAS
			{ 0x03, 0x07, 0x03, 0x00, 0x00, 0x02, 0x02, 0x00 },   	// MSAS
			{ 0x03, 0x07, 0x03, 0x00, 0x80, 0x01, 0x00, 0x00 },   	// GAGAN
			{ 0x02, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 }		// Disabled
    };
#endif

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

	if (!pifGpsUblox_Init(&s_gps_ublox, PIF_ID_AUTO)) return;
	pifGpsUblox_AttachComm(&s_gps_ublox, &s_comm_gps);
#ifdef NMEA
	s_gps_ublox._gps.evt_nmea_msg_id = PIF_GPS_NMEA_MSG_ID_GGA;
	if (!pifGpsUblox_SetPubxConfig(&s_gps_ublox, 1, 0x03, 0x03, 38400, TRUE)) return;
	pifTaskManager_YieldMs(50);
	Serial1.begin(38400);
#endif
#ifdef UBX
	s_gps_ublox.evt_ubx_receive = _evtGpsUbxReceive;
	if (!pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_PRT, sizeof(kCfgPrt), (uint8_t*)kCfgPrt, TRUE)) return;
	pifTaskManager_YieldMs(50);
	actGpsSetBaudrate(115200);
	for (uint8_t i = 0; i < sizeof(kCfgMsg) / sizeof(kCfgMsg[0]); i++) {
		if (!pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_MSG, sizeof(kCfgMsg[i]), (uint8_t*)kCfgMsg[i], TRUE)) return;
	}
	if (!pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_RATE, sizeof(kCfgRate), (uint8_t*)kCfgRate, TRUE)) return;
	if (!pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_NAV5, sizeof(kCfgNav5), (uint8_t*)kCfgNav5, TRUE)) return;
	if (!pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_SBAS, sizeof(kCfgSbas[0]), (uint8_t*)kCfgSbas[0], TRUE)) return;		// 0 = Auto
	s_gps_ublox.evt_ubx_receive = NULL;
#endif
	s_gps_ublox._gps.evt_receive = _evtGpsReceive;

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
