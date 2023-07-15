#include "linker.h"

#include "communication/pif_i2c.h"
#include "core/pif_log.h"
#include "display/pif_led.h"
#include "gps/pif_gps_nmea.h"


PifUart g_uart_log;
PifI2cPort g_i2c_port;
PifTimerManager g_timer_1ms;

int g_print_data = 0;

static PifGpsNmea s_gps_nmea;
static PifLed s_led_l;

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

	pifLed_PartToggle(&s_led_l, 1 << 1);

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

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;					// 1000us

	if (!pifUart_Init(&g_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_log, TM_PERIOD_MS, 1, "UartLog")) return;				// 1ms
	if (!pifUart_AllocRxBuffer(&g_uart_log, 64, 100)) return;								// 64bytes, 100%
	if (!pifUart_AllocTxBuffer(&g_uart_log, 128)) return;									// 128bytes
	g_uart_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachUart(&g_uart_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;										// 500ms
    pifLed_SBlinkOn(&s_led_l, 1 << 0);

    if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 1, 30)) return;
    g_i2c_port.act_read = actI2cRead;
    g_i2c_port.act_write = actI2cWrite;

	if (!pifGpsNmea_Init(&s_gps_nmea, PIF_ID_AUTO)) return;
	if (!pifGpsNmea_AttachI2c(&s_gps_nmea, &g_i2c_port, 0x42, 250, TRUE, NULL)) return;		// 0x42 : Ublox I2c addrress, 500ms
	s_gps_nmea._gps.evt_receive = _evtGpsReceive;
	s_gps_nmea._gps.evt_nmea_receive = _evtGpsNmeaReceive;
	if (!pifGps_SetEventNmeaText(&s_gps_nmea._gps, _evtGpsNmeaText)) return;

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
	s_booting = TRUE;
}
