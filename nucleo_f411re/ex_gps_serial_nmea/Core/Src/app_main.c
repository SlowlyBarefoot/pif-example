#include "app_main.h"
#include "main.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "gps/pif_gps_nmea.h"


PifComm g_comm_log;
PifComm g_comm_gps;
PifTimerManager g_timer_1ms;

static int s_print_data = 1;
static PifGpsNmea s_gps_nmea;
static PifLed s_led_l;

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

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;			// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, "CommLog")) return;		// 1ms
	if (!pifComm_AllocRxBuffer(&g_comm_log, 64, 100)) return;						// 64bytes, 100%
	if (!pifComm_AllocTxBuffer(&g_comm_log, 128)) return;							// 128bytes
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

	if (!pifComm_Init(&g_comm_gps, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_gps, TM_PERIOD_MS, 1, "CommGPS")) return;		// 1ms
	if (!pifComm_AllocRxBuffer(&g_comm_gps, 256, 100)) return;						// 256bytes, 100%
	if (!pifComm_AllocTxBuffer(&g_comm_gps, 32)) return;							// 32bytes
	g_comm_gps.act_start_transfer = actGpsStartTransfer;

	if (!pifGpsNmea_Init(&s_gps_nmea, PIF_ID_AUTO)) return;
	pifGpsNmea_AttachComm(&s_gps_nmea, &g_comm_gps);
	if (!pifGps_SetEventNmeaText(&s_gps_nmea._gps, _evtGpsNmeaText)) return;
	s_gps_nmea._gps.evt_nmea_receive = _evtGpsNmeaReceive;
	s_gps_nmea._gps.evt_receive = _evtGpsReceive;



    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;								// 500ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
