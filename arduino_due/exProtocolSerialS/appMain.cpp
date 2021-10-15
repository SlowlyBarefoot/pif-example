#include "appMain.h"
#include "exProtocolSerialS.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifProtocol.h"
#include "pifSensorSwitch.h"


#define SWITCH_COUNT            2


PifPulse *g_pstTimer1ms = NULL;

static PifComm *s_pstSerial = NULL;
static PifProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion20(PifProtocolPacket *pstPacket);
static void _fnProtocolQuestion21(PifProtocolPacket *pstPacket);

static void _fnProtocolResponse30(PifProtocolPacket *pstPacket);
static void _fnProtocolResponse31(PifProtocolPacket *pstPacket);

const PifProtocolQuestion stProtocolQuestions[] = {
		{ 0x20, PF_ANSWER_YES | PF_LOG_PRINT_YES, _fnProtocolQuestion20 },
		{ 0x21, PF_ANSWER_NO | PF_LOG_PRINT_YES, _fnProtocolQuestion21 },
		{ 0, PF_DEFAULT, NULL }
};

const PifProtocolRequest stProtocolRequests[] = {
		{ 0x30, PF_RESPONSE_YES | PF_LOG_PRINT_YES, _fnProtocolResponse30, 3, 300 },
		{ 0x31, PF_RESPONSE_NO | PF_LOG_PRINT_YES, _fnProtocolResponse31, 3, 300 },
		{ 0, PF_DEFAULT, NULL, 0, 0 }
};

static struct {
	PifSensor *pstPushSwitch;
	uint8_t ucDataCount;
	uint8_t ucData[8];
	PifSensorSwitchFilter stPushSwitchFilter;
} s_stProtocolTest[SWITCH_COUNT] = {
		{ NULL, 0, },
		{ NULL, 0, }
};


static void _fnProtocolPrint(PifProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_INFO, "%s: CNT=%u", pcName, pstPacket->data_count);
		if (pstPacket->data_count) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (uint16_t i = 0; i < pstPacket->data_count; i++) {
				pifLog_Printf(LT_NONE, " %u", pstPacket->p_data[i]);
			}
		}
	}
	else {
		pifLog_Printf(LT_INFO, pcName);
	}
}

static void _fnCompareData(PifProtocolPacket *pstPacket, uint8_t ucIndex)
{
	uint16_t i;

	if (pstPacket->data_count == s_stProtocolTest[ucIndex].ucDataCount) {
		for (i = 0; i < pstPacket->data_count; i++) {
			if (pstPacket->p_data[i] != s_stProtocolTest[ucIndex].ucData[i]) break;
		}
		if (i < pstPacket->data_count) {
			pifLog_Printf(LT_INFO, "Different data");
		}
		else {
			pifLog_Printf(LT_INFO, "Same data");
		}
	}
	else {
		pifLog_Printf(LT_ERROR, "Different count: %u != %u", s_stProtocolTest[ucIndex].ucDataCount, pstPacket->data_count);
	}
}

static void _fnProtocolQuestion20(PifProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 0);
	_fnProtocolPrint(pstPacket, "Question20");

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].flags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Question20: Error=%d", pif_error);
	}
}

static void _fnProtocolQuestion21(PifProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 1);
	_fnProtocolPrint(pstPacket, "Question21");
}

static void _fnProtocolResponse30(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response30");
}

static void _fnProtocolResponse31(PifProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_INFO, "Response31: ACK");
}

static void _evtProtocolError(PifId usPifId)
{
	pifLog_Printf(LT_ERROR, "ProtocolError DC=%d", usPifId);
}

static void _evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	uint8_t index = usPifId - PIF_ID_SWITCH;

	(void)pvIssuer;

	if (usLevel) {
		s_stProtocolTest[index].ucDataCount = rand() % 8;
		for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) s_stProtocolTest[index].ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(s_pstProtocol, &stProtocolRequests[index], s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
			pifLog_Printf(LT_ERROR, "PushSwitchChange(%d): DC=%d E=%d", index, s_pstProtocol->_id, pif_error);
		}
		else {
			pifLog_Printf(LT_INFO, "PushSwitchChange(%d): DC=%d CNT=%u", index, s_pstProtocol->_id, s_stProtocolTest[index].ucDataCount);
			if (s_stProtocolTest[index].ucDataCount) {
				pifLog_Printf(LT_NONE, "\nData:");
				for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
					pifLog_Printf(LT_NONE, " %u", s_stProtocolTest[index].ucData[i]);
				}
			}
		}
	}
}

void appSetup()
{
	int i;
	PifComm *pstCommLog;
	PifLed *pstLedL;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);															// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;										// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;										// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;																// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    for (i = 0; i < SWITCH_COUNT; i++) {
    	s_stProtocolTest[i].pstPushSwitch = pifSensorSwitch_Create(PIF_ID_SWITCH + i, 0);
		if (!s_stProtocolTest[i].pstPushSwitch) return;
	    if (!pifSensorSwitch_AttachTask(s_stProtocolTest[i].pstPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;	// 10m
		pifSensor_AttachAction(s_stProtocolTest[i].pstPushSwitch, actPushSwitchAcquire);
		pifSensor_AttachEvtChange(s_stProtocolTest[i].pstPushSwitch, _evtPushSwitchChange, NULL);
	    if (!pifSensorSwitch_AttachFilter(s_stProtocolTest[i].pstPushSwitch, PIF_SENSOR_SWITCH_FILTER_COUNT, 7, &s_stProtocolTest[i].stPushSwitchFilter)) return;
    }

    s_pstSerial = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstSerial) return;
    if (!pifComm_AttachTask(s_pstSerial, TM_PERIOD_MS, 1, TRUE)) return;										// 1ms
	pifComm_AttachActReceiveData(s_pstSerial, actSerialReceiveData);
	pifComm_AttachActSendData(s_pstSerial, actSerialSendData);

    s_pstProtocol = pifProtocol_Create(PIF_ID_AUTO, g_pstTimer1ms, PT_SMALL, stProtocolQuestions);
    if (!s_pstProtocol) return;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    s_pstProtocol->evt_error = _evtProtocolError;
}
