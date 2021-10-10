#include "appMain.h"
#include "exGpsSerialNmea.h"

#include "pifGpsNmea.h"
#include "pifLed.h"
#include "pifLog.h"


PifPulse *g_pstTimer1ms = NULL;
BOOL g_bPrintRawData = FALSE;

static PIF_stComm *s_pstCommGps = NULL;
static PIF_stGpsNmea *s_pstGpsNmea = NULL;
static PifLed *s_pstLedL = NULL;

static int _cmdPrintRawData(int argc, char *argv[]);
static int _cmdRequest(int argc, char *argv[]);

const PIF_stLogCmdEntry c_psCmdTable[] = {
	{ "raw", _cmdPrintRawData, "\nPrint RawData" },
	{ "req", _cmdRequest, "\nRequest" },

	{ NULL, NULL, NULL }
};


static int _cmdPrintRawData(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_enNone, "\n  Print RawData: %d", g_bPrintRawData);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		switch (argv[1][0]) {
		case '0':
		case 'F':
		case 'f':
			g_bPrintRawData = FALSE;
			break;

		case '1':
		case 'T':
		case 't':
			g_bPrintRawData = TRUE;
			break;

		default:
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _cmdRequest(int argc, char *argv[])
{
	int command;

	if (argc > 2) {
		command = atoi(argv[1]);
		switch (command) {
		case NMEA_MESSAGE_ID_GBQ:
			if (!pifGpsNmea_PollRequestGBQ(s_pstGpsNmea, argv[2])) {
				pifLog_Printf(LT_enError, "Error: %u", pif_error);
			}
			break;

		case NMEA_MESSAGE_ID_GLQ:
			if (!pifGpsNmea_PollRequestGLQ(s_pstGpsNmea, argv[2])) {
				pifLog_Printf(LT_enError, "Error: %u", pif_error);
			}
			break;

		case NMEA_MESSAGE_ID_GNQ:
			if (!pifGpsNmea_PollRequestGNQ(s_pstGpsNmea, argv[2])) {
				pifLog_Printf(LT_enError, "Error: %u", pif_error);
			}
			break;

		case NMEA_MESSAGE_ID_GPQ:
			if (!pifGpsNmea_PollRequestGPQ(s_pstGpsNmea, argv[2])) {
				pifLog_Printf(LT_enError, "Error: %u", pif_error);
			}
			break;

		default:
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtGpsNmeaText(PIF_stGpsNmeaTxt *pstTxt)
{
	const char *acType[4] = { "Error", "Warning", "Notice", "User" };

	pifLog_Printf(LT_enInfo, "Text: Total=%u Num=%u Type=%s:%s", pstTxt->ucTotal, pstTxt->ucNum, acType[pstTxt->ucType], pstTxt->acText);
}

static void _evtGpsReceive(PIF_stGps *pstOwner)
{
	PIF_stDegMin stLatDegMin, stLonDegMin;
	PIF_stDegMinSec stLatDegMinSec, stLonDegMinSec;

	pifLed_EachToggle(s_pstLedL, 1);

	pifGps_ConvertLatitude2DegMin(pstOwner, &stLatDegMin);
	pifGps_ConvertLongitude2DegMin(pstOwner, &stLonDegMin);

	pifGps_ConvertLatitude2DegMinSec(pstOwner, &stLatDegMinSec);
	pifGps_ConvertLongitude2DegMinSec(pstOwner, &stLonDegMinSec);

	if (!g_bPrintRawData) {
		pifLog_Printf(LT_enInfo, "UTC Date Time: %4u/%2u/%2u %2u:%2u:%2u.%3u",
				2000 + pstOwner->_stDateTime.year, pstOwner->_stDateTime.month, pstOwner->_stDateTime.day,
				pstOwner->_stDateTime.hour, pstOwner->_stDateTime.minute, pstOwner->_stDateTime.second,
				pstOwner->_stDateTime.millisecond);
		pifLog_Printf(LT_enInfo, "Longitude: %f` - %u`%f' - %u`%u'%f\"",
				pstOwner->_dCoordDeg[GPS_LON], stLonDegMin.usDegree, stLonDegMin.dMinute,
				stLonDegMinSec.usDegree, stLonDegMinSec.usMinute, stLonDegMinSec.dSecond);
		pifLog_Printf(LT_enInfo, "Latitude: %f` - %u`%f' - %u`%u'%f\"",
				pstOwner->_dCoordDeg[GPS_LAT], stLatDegMin.usDegree, stLatDegMin.dMinute,
				stLatDegMinSec.usDegree, stLatDegMinSec.usMinute, stLatDegMinSec.dSecond);
		pifLog_Printf(LT_enInfo, "NumSat: %u", pstOwner->_ucNumSat);
		pifLog_Printf(LT_enInfo, "Altitude: %f m", pstOwner->_dAltitude);
		pifLog_Printf(LT_enInfo, "Speed: %f knots %f m/s %f km/h", pstOwner->_dSpeedN, pifGps_ConvertKnots2MpS(pstOwner->_dSpeedN), pstOwner->_dSpeedK);
		pifLog_Printf(LT_enInfo, "Ground Course: %f deg", pstOwner->_dGroundCourse);
		pifLog_Printf(LT_enInfo, "Fix: %u", pstOwner->_ucFix);
	}
	pifLog_Printf(LT_enNone, "\n");
}

void appSetup()
{
	PIF_stComm *pstCommLog;

	pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;	// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	pifComm_AttachActReceiveData(pstCommLog, actLogReceiveData);
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 2, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;						// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

	s_pstCommGps = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstCommGps) return;
    if (!pifComm_AttachTask(s_pstCommGps, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	pifComm_AttachActReceiveData(s_pstCommGps, actGpsReceiveData);
	pifComm_AttachActSendData(s_pstCommGps, actGpsSendData);

	s_pstGpsNmea = pifGpsNmea_Create(PIF_ID_AUTO);
	if (!s_pstGpsNmea) return;
	if (!pifGpsNmea_SetProcessMessageId(s_pstGpsNmea, 4, NMEA_MESSAGE_ID_GGA, NMEA_MESSAGE_ID_TXT, NMEA_MESSAGE_ID_VTG, NMEA_MESSAGE_ID_ZDA)) return;
	pifGpsNmea_SetEventMessageId(s_pstGpsNmea, NMEA_MESSAGE_ID_GGA);
	pifGpsNmea_AttachComm(s_pstGpsNmea, s_pstCommGps);
	pifGpsNmea_AttachEvtText(s_pstGpsNmea, _evtGpsNmeaText);
	pifGps_AttachEvent(&s_pstGpsNmea->_stGps, _evtGpsReceive);

    if (!pifLog_AttachTask(TM_PERIOD_MS, 20, TRUE)) return;				// 20ms
}
