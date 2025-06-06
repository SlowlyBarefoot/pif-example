#include "app_main.h"

#include "gps/pif_gps_ublox.h"

#include <string.h>

//#define NMEA
#define UBX

//#define WAITING	100
#define WAITING		0

#define BLOCKING	FALSE

PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_gps;

int g_print_data = 0;

static PifGpsUblox s_gps_ublox;
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

static struct {
    PifDateTime _utc;
    double _coord_deg[2];			// latitude, longitude	- unit: degree
	double _altitude;       		// altitude      		- unit: meter
	double _ground_speed;      		// ground speed         - unit: cm/s
	double _ground_course;			//                   	- unit: degree
	uint8_t _num_sat;
	BOOL _fix;
    BOOL __next_fix;
	uint32_t _horizontal_acc;       // Horizontal accuracy estimate (mm)
	uint32_t _vertical_acc;         // Vertical accuracy estimate (mm)
	uint8_t _sv_num_ch;
	uint8_t _sv_chn[PIF_GPS_SV_MAXSATS];     // Channel number
	uint8_t _sv_svid[PIF_GPS_SV_MAXSATS];    // Satellite ID
	uint8_t _sv_quality[PIF_GPS_SV_MAXSATS]; // Bitfield Qualtity
	uint8_t _sv_cno[PIF_GPS_SV_MAXSATS];     // Carrier to Noise Ratio (Signal Strength)
} s_gps_data;


#ifdef NMEA

static void _evtGpsNmeaFrame(char* p_frame)
{
	pifLog_Print(LT_NONE, p_frame);
}

static BOOL _evtGpsNmeaReceive(PifGps *p_owner, PifGpsNmeaMsgId msg_id)
{
	const char *acType[4] = { "Error", "Warning", "Notice", "User" };

	switch (msg_id) {
	case PIF_GPS_NMEA_MSG_ID_GGA:
		s_gps_data._utc = p_owner->_gga.utc;
		s_gps_data._coord_deg[PIF_GPS_LAT] = p_owner->_gga.lat;
		s_gps_data._coord_deg[PIF_GPS_LON] = p_owner->_gga.lon;
		s_gps_data._fix = p_owner->_gga.quality;
		s_gps_data._num_sat = p_owner->_gga.num_sv;
		s_gps_data._altitude = p_owner->_gga.alt;
		return TRUE;

	case PIF_GPS_NMEA_MSG_ID_RMC:
		s_gps_data._utc = p_owner->_rmc.utc;
		s_gps_data._ground_speed = p_owner->_rmc.spd;
		s_gps_data._ground_course = p_owner->_rmc.cog;
		break;

	case PIF_GPS_NMEA_MSG_ID_TXT:
		pifLog_Printf(LT_NONE, "%s] %s\n", acType[p_owner->_txt.msg_type], p_owner->_txt.text);
		break;
	}
	return FALSE;
}

static uint32_t _taskNmeaSetup(PifTask *p_task)
{
	uint32_t baudrates[] = { 115200, 57600, 38400, 19200, 9600 };
	uint8_t n;
	uint16_t delay = 100;
	static uint8_t step = 0, retry;

	pifLog_Printf(LT_INFO, "NMEA: Step=%x", step);

	switch (step & 0xF0) {
	case 0x10:
		(*g_uart_gps.act_set_baudrate)(&g_uart_gps, baudrates[step - 0x10]);
		step += 0x10;
		retry = 2;
		break;

	case 0x20:
		if (pifGpsUblox_SetPubxConfig(&s_gps_ublox, 1, 0x03, 0x03, s_baudrate, TRUE, 100)) {
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d Retry=%d", GUCI_CFG, GUMI_CFG_PRT, retry);
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

	default:
		switch (step) {
		case 0:
			step = 0x10;
			delay = 5000;
			break;

		case 0x30:
			(*g_uart_gps.act_set_baudrate)(&g_uart_gps, s_baudrate);
			step = 0x40;
			delay = 500;
			break;

		case 0x40:
			p_task->pause = TRUE;
			s_booting = TRUE;
			break;
		}
		break;
	}
    return delay * 1000;
}

#endif

#ifdef UBX

static BOOL _evtGpsUbxReceive(PifGpsUblox* p_owner, PifGpsUbxPacket* p_packet)
{
	int i;

    switch (p_packet->class_id) {
    case GUCI_ACK:
    	pifLog_Printf(LT_INFO, "Ack: mid=%u, cid=%u mid=%u", p_packet->msg_id, p_packet->ack.cls_id, p_packet->ack.msg_id);
    	break;

    case GUCI_NAV:
    	switch (p_packet->msg_id) {
        case GUMI_NAV_POSLLH:
        	s_gps_data._coord_deg[PIF_GPS_LON] = p_packet->posllh.lon / 10000000.0;
        	s_gps_data._coord_deg[PIF_GPS_LAT] = p_packet->posllh.lat / 10000000.0;
        	s_gps_data._altitude = p_packet->posllh.h_msl / 1000.0;
        	s_gps_data._horizontal_acc = p_packet->posllh.h_acc;
        	s_gps_data._vertical_acc = p_packet->posllh.v_acc;
            s_gps_data._fix = s_gps_data.__next_fix;
		    return TRUE;

        case GUMI_NAV_PVT:
           	s_gps_data._utc.year = 20 + p_packet->pvt.year - 2000;
        	s_gps_data._utc.month = p_packet->pvt.month;
        	s_gps_data._utc.day = p_packet->pvt.day;
        	s_gps_data._utc.hour = p_packet->pvt.hour;
        	s_gps_data._utc.minute = p_packet->pvt.min;
        	s_gps_data._utc.second = p_packet->pvt.sec;
        	s_gps_data._utc.millisecond = p_packet->pvt.nano / 1000000UL;
            s_gps_data._num_sat = p_packet->pvt.num_sv;
            break;

        case GUMI_NAV_SOL:
        	s_gps_data.__next_fix = (p_packet->sol.flags & NAV_STATUS_FIX_VALID) && (p_packet->sol.gps_fix == FIX_3D);
            if (!s_gps_data.__next_fix)
               	s_gps_data._fix = FALSE;
            s_gps_data._num_sat = p_packet->sol.num_sv;
            break;

        case GUMI_NAV_STATUS:
        	s_gps_data.__next_fix = (p_packet->status.flags & NAV_STATUS_FIX_VALID) && (p_packet->status.gps_fix == FIX_3D);
            if (!s_gps_data.__next_fix)
              	s_gps_data._fix = FALSE;
            break;

    	case GUMI_NAV_SVINFO:
			s_gps_data._sv_num_ch = p_packet->sv_info.num_ch;
			if (s_gps_data._sv_num_ch > PIF_GPS_SV_MAXSATS)
				s_gps_data._sv_num_ch = PIF_GPS_SV_MAXSATS;
			for (i = 0; i < s_gps_data._sv_num_ch; i++) {
				s_gps_data._sv_chn[i] = p_packet->sv_info.channel[i].chn;
				s_gps_data._sv_svid[i] = p_packet->sv_info.channel[i].svid;
				s_gps_data._sv_quality[i] = p_packet->sv_info.channel[i].quality;
				s_gps_data._sv_cno[i] = p_packet->sv_info.channel[i].cno;
			}
			break;
        case GUMI_NAV_TIMEUTC:
           	if (p_packet->time_utc.valid & 4) {
           		s_gps_data._utc.year = p_packet->time_utc.year - 2000;
           		s_gps_data._utc.month = p_packet->time_utc.month;
           		s_gps_data._utc.day = p_packet->time_utc.day;
           		s_gps_data._utc.hour = p_packet->time_utc.hour;
           		s_gps_data._utc.minute = p_packet->time_utc.min;
           		s_gps_data._utc.second = p_packet->time_utc.sec;
           		s_gps_data._utc.millisecond = p_packet->time_utc.nano / 1000000UL;
    		}
			break;

        case GUMI_NAV_VELNED:
        	s_gps_data._ground_speed = p_packet->velned.speed;
        	s_gps_data._ground_course = p_packet->velned.heading / 100000.0;
            break;

		}
		break;
    }
	return FALSE;
}

static uint32_t _taskUbloxSetup(PifTask *p_task)
{
	uint32_t baudrates[] = { 115200, 57600, 38400, 19200, 9600 };
    const uint8_t kCfgMsgNmea[][3] = {
			{ GUCI_NMEA_STD, GUMI_NMEA_VTG, 0x00 }, // Course over ground and Ground speed
			{ GUCI_NMEA_STD, GUMI_NMEA_GSV, 0x00 }, // GNSS Satellites in View
			{ GUCI_NMEA_STD, GUMI_NMEA_GLL, 0x00 }, // Latitude and longitude, with time of position fix and status
			{ GUCI_NMEA_STD, GUMI_NMEA_GGA, 0x00 }, // Global positioning system fix data
			{ GUCI_NMEA_STD, GUMI_NMEA_GSA, 0x00 }, // GNSS DOP and Active Satellites
			{ GUCI_NMEA_STD, GUMI_NMEA_RMC, 0x00 }	// Recommended Minimum data
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
	static uint8_t step = 0, retry;

	pifLog_Printf(LT_INFO, "UBX: Step=%x RS=%d", step, s_gps_ublox._request_state);

	switch (step & 0xF0) {
	case 0x10:
		(*g_uart_gps.act_set_baudrate)(&g_uart_gps, baudrates[step - 0x10]);
		step += 0x10;
		retry = 2;
		delay = 200;
		break;

	case 0x20:
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Retry=%d", GUCI_CFG, GUMI_CFG_PRT, retry);
		if (pifGpsUblox_SetPubxConfig(&s_gps_ublox, 1, 0x03, 0x01, s_baudrate, BLOCKING, 0)) {
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
		n = (step - 0x40) >> 1;
#if WAITING > 0
		pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d-%d: Result=%d", GUCI_CFG, GUMI_CFG_MSG, n, s_gps_ublox._request_state);
		pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_MSG, sizeof(kCfgMsgNmea[n]), (uint8_t*)kCfgMsgNmea[n], BLOCKING, WAITING);
		if (s_gps_ublox._request_state == GURS_ACK) {
			n++;
			if (n < sizeof(kCfgMsgNmea) / sizeof(kCfgMsgNmea[0])) step += 2; else step = 0x50;
			delay = 100;
		}
		else {
			step = 0x10;
			delay = 500;
		}
#else
		if (step & 1) {
			switch (s_gps_ublox._request_state) {
			case GURS_SEND:
				break;

			case GURS_ACK:
				n++;
				if (n < sizeof(kCfgMsgNmea) / sizeof(kCfgMsgNmea[0])) step++; else step = 0x50;
				delay = 100;
				break;

			default:
				step = 0x10;
				delay = 500;
				break;
			}
		}
		else {
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d-%d: Result=%d", GUCI_CFG, GUMI_CFG_MSG, n, s_gps_ublox._request_state);
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_MSG, sizeof(kCfgMsgNmea[n]), (uint8_t*)kCfgMsgNmea[n], BLOCKING, WAITING);
			step++;
		}
#endif
		break;

	case 0x50:
		n = (step - 0x50) >> 1;
#if WAITING > 0
		pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d-%d: Result=%d", GUCI_CFG, GUMI_CFG_MSG, n, s_gps_ublox._request_state);
		pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_MSG, sizeof(kCfgMsgNav[n]), (uint8_t*)kCfgMsgNav[n], BLOCKING, WAITING);
		if (s_gps_ublox._request_state == GURS_ACK) {
			n++;
			if (n < sizeof(kCfgMsgNav) / sizeof(kCfgMsgNav[0])) step += 2; else step = 0x60;
			delay = 100;
		}
		else {
			step = 0x10;
			delay = 500;
		}
#else
		if (step & 1) {
			switch (s_gps_ublox._request_state) {
			case GURS_SEND:
				break;

			case GURS_ACK:
				n++;
				if (n < sizeof(kCfgMsgNav) / sizeof(kCfgMsgNav[0])) step++; else step = 0x60;
				delay = 100;
				break;

			default:
				step = 0x10;
				delay = 500;
				break;
			}
		}
		else {
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d-%d: Result=%d", GUCI_CFG, GUMI_CFG_MSG, n, s_gps_ublox._request_state);
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_MSG, sizeof(kCfgMsgNav[n]), (uint8_t*)kCfgMsgNav[n], BLOCKING, WAITING);
			step++;
		}
#endif
		break;

	default:
		switch (step) {
		case 0:
			step = 0x10;
			delay = 5000;
			break;

		case 0x30:
			(*g_uart_gps.act_set_baudrate)(&g_uart_gps, s_baudrate);
			step = 0x40;
			delay = 500;
			break;

		case 0x60:
			p_rate = (uint16_t*)kCfgRate;
			*p_rate = 1000;	// 1000 = 1Hz
#if WAITING > 0
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_RATE, s_gps_ublox._request_state);
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_RATE, sizeof(kCfgRate), (uint8_t*)kCfgRate, BLOCKING, WAITING);
			if (s_gps_ublox._request_state == GURS_ACK) {
				step += 2;
				delay = 100;
			}
			else {
				step = 0x10;
				delay = 500;
			}
#else
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_RATE, s_gps_ublox._request_state);
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_RATE, sizeof(kCfgRate), (uint8_t*)kCfgRate, BLOCKING, WAITING);
			step++;
#endif
			break;

		case 0x61:
			switch (s_gps_ublox._request_state) {
			case GURS_SEND:
				break;

			case GURS_ACK:
				step++;
				delay = 100;
				break;

			default:
				step = 0x10;
				delay = 500;
				break;
			}
			break;

		case 0x62:
#if WAITING > 0
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_NAV5, s_gps_ublox._request_state);
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_NAV5, sizeof(kCfgNav5), (uint8_t*)kCfgNav5, BLOCKING, WAITING);
			if (s_gps_ublox._request_state == GURS_ACK) {
				step += 2;
				delay = 100;
			}
			else {
				step = 0x10;
				delay = 500;
			}
#else
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_NAV5, s_gps_ublox._request_state);
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_NAV5, sizeof(kCfgNav5), (uint8_t*)kCfgNav5, BLOCKING, WAITING);
			step++;
#endif
			break;

		case 0x63:
			switch (s_gps_ublox._request_state) {
			case GURS_SEND:
				break;

			case GURS_ACK:
				step++;
				delay = 100;
				break;

			default:
				step = 0x10;
				delay = 500;
				break;
			}
			break;

		case 0x64:
#if WAITING > 0
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_SBAS, s_gps_ublox._request_state);
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_SBAS, sizeof(kCfgSbas[sbas]), (uint8_t*)kCfgSbas[sbas], BLOCKING, WAITING);
			if (s_gps_ublox._request_state == GURS_ACK) {
				step += 2;
				delay = 100;
			}
			else {
				step = 0x10;
				delay = 500;
			}
#else
			pifLog_Printf(LT_INFO, "ClassId=%d MsgId=%d: Result=%d", GUCI_CFG, GUMI_CFG_SBAS, s_gps_ublox._request_state);
			pifGpsUblox_SendUbxMsg(&s_gps_ublox, GUCI_CFG, GUMI_CFG_SBAS, sizeof(kCfgSbas[sbas]), (uint8_t*)kCfgSbas[sbas], BLOCKING, WAITING);
			step++;
#endif
			break;

		case 0x65:
			switch (s_gps_ublox._request_state) {
			case GURS_SEND:
				break;

			case GURS_ACK:
				step++;
				delay = 100;
				break;

			default:
				step = 0x10;
				delay = 500;
				break;
			}
			break;

		case 0x66:
			p_task->pause = TRUE;
			s_booting = TRUE;
			break;
		}
		break;
	}
    return delay * 1000;
}

#endif

static void _evtGpsReceive(PifGps *p_owner)
{
	PifDegMin lat_deg_min, lon_deg_min;
	PifDegMinSec lat_deg_min_sec, lon_deg_min_sec;

	pifLed_PartToggle(&g_led_l, 1 << 1);

	if (!s_booting) return;

	pifGps_ConvertDegree2DegMin(s_gps_data._coord_deg[PIF_GPS_LAT], &lat_deg_min);
	pifGps_ConvertDegree2DegMin(s_gps_data._coord_deg[PIF_GPS_LON], &lon_deg_min);

	pifGps_ConvertDegree2DegMinSec(s_gps_data._coord_deg[PIF_GPS_LAT], &lat_deg_min_sec);
	pifGps_ConvertDegree2DegMinSec(s_gps_data._coord_deg[PIF_GPS_LON], &lon_deg_min_sec);

	pifLog_Printf(LT_INFO, "UTC Date Time: %4u/%2u/%2u %2u:%2u:%2u.%3u",
			2000 + s_gps_data._utc.year, s_gps_data._utc.month, s_gps_data._utc.day,
			s_gps_data._utc.hour, s_gps_data._utc.minute, s_gps_data._utc.second, s_gps_data._utc.millisecond);
	pifLog_Printf(LT_INFO, "Longitude: %f` - %u`%f' - %u`%u'%f\"",
			s_gps_data._coord_deg[PIF_GPS_LON], lon_deg_min.degree, lon_deg_min.minute,
			lon_deg_min_sec.degree, lon_deg_min_sec.minute, lon_deg_min_sec.second);
	pifLog_Printf(LT_INFO, "Latitude: %f` - %u`%f' - %u`%u'%f\"",
			s_gps_data._coord_deg[PIF_GPS_LAT], lat_deg_min.degree, lat_deg_min.minute,
			lat_deg_min_sec.degree, lat_deg_min_sec.minute, lat_deg_min_sec.second);
	pifLog_Printf(LT_INFO, "NumSat: %u", s_gps_data._num_sat);
	pifLog_Printf(LT_INFO, "Altitude: %f m", s_gps_data._altitude);
	pifLog_Printf(LT_INFO, "Speed: %f cm/s", s_gps_data._ground_speed);
	pifLog_Printf(LT_INFO, "Ground Course: %f deg", s_gps_data._ground_course);
	pifLog_Printf(LT_INFO, "Fix: %u", s_gps_data._fix);
	pifLog_Printf(LT_INFO, "Acc: Hor %lu mm Ver %lu mm", s_gps_data._horizontal_acc, s_gps_data._vertical_acc);
	pifLog_Printf(LT_INFO, "SvInfo: ch=%u", s_gps_data._sv_num_ch);
	for (int i = 0; i < s_gps_data._sv_num_ch; i++) {
		pifLog_Printf(LT_INFO, "   ch=%u svid=%u quality=%u cno=%u", s_gps_data._sv_chn[i], s_gps_data._sv_svid[i], s_gps_data._sv_quality[i], s_gps_data._sv_cno[i]);
	}
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

BOOL appSetup(uint32_t baurdate)
{
	s_baudrate = baurdate;

    if (!pifLog_UseCommand(32, c_psCmdTable, "\nDebug> ")) return FALSE;								// 32bytes

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;												// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    memset(&s_gps_data, 0, sizeof(s_gps_data));

	if (!pifGpsUblox_Init(&s_gps_ublox, PIF_ID_AUTO)) return FALSE;
	pifGpsUblox_AttachUart(&s_gps_ublox, &g_uart_gps);
	s_gps_ublox._gps.evt_receive = _evtGpsReceive;
#ifdef NMEA
	s_gps_ublox._gps.evt_nmea_receive = _evtGpsNmeaReceive;
	if (!pifTaskManager_Add(TM_PERIOD, 100000, _taskNmeaSetup, NULL, TRUE)) return FALSE;				// 100ms
#endif
#ifdef UBX
	s_gps_ublox.evt_ubx_receive = _evtGpsUbxReceive;
	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 100000, _taskUbloxSetup, NULL, TRUE)) return FALSE;	// 100ms
#endif
	return TRUE;
}
