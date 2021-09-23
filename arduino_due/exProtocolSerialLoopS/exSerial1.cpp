#include "exProtocolSerialLoopS.h"
#include "appMain.h"

#include "pifLog.h"
#include "pifProtocol.h"
#include "pifSensorSwitch.h"


PIF_stComm *g_pstSerial1 = NULL;

static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolAnswer20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolAnswer21(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse31(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions1[] = {
		{ 0x20, PF_enAnswer_Yes | PF_enLogPrint_Yes, _fnProtocolAnswer20 },
		{ 0x21, PF_enAnswer_No | PF_enLogPrint_Yes, _fnProtocolAnswer21 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequests1[] = {
		{ 0x30, PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolResponse30, 3, 300 },
		{ 0x31, PF_enResponse_No | PF_enLogPrint_Yes, _fnProtocolResponse31, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};

static struct {
	PIF_stSensor *pstPushSwitch;
	uint8_t ucDataCount;
	uint8_t ucData[8];
	PIF_stSensorSwitchFilter stPushSwitchFilter;
} s_stProtocolTest[SWITCH_COUNT] = {
		{ NULL, 0, },
		{ NULL, 0, }
};


static void _fnProtocolPrint(PIF_stProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_enInfo, "%s: CNT=%u", pcName, pstPacket->usDataCount);
#ifdef PRINT_PACKET_DATA
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
		}
#endif
	}
	else {
		pifLog_Printf(LT_enInfo, pcName);
	}
}

static void _fnCompareData(PIF_stProtocolPacket *pstPacket, uint8_t ucIndex)
{
	uint16_t i;

	if (pstPacket->usDataCount == s_stProtocolTest[ucIndex].ucDataCount) {
		for (i = 0; i < pstPacket->usDataCount; i++) {
			if (pstPacket->pucData[i] != s_stProtocolTest[ucIndex].ucData[i]) break;
		}
		if (i < pstPacket->usDataCount) {
			pifLog_Printf(LT_enInfo, "Different data");
		}
	}
	else {
		pifLog_Printf(LT_enError, "Different count: %u != %u", s_stProtocolTest[ucIndex].ucDataCount, pstPacket->usDataCount);
	}
}

static void _fnProtocolAnswer20(PIF_stProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 0);
	_fnProtocolPrint(pstPacket, "Answer20");

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions1[0].enFlags, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "Answer20: Error=%d", pif_enError);
	}
}

static void _fnProtocolAnswer21(PIF_stProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 1);
	_fnProtocolPrint(pstPacket, "Answer21");
}

static void _fnProtocolResponse30(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response30");
}

static void _fnProtocolResponse31(PIF_stProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_enInfo, "Response31");
}

static void _evtProtocolError(PIF_usId usPifId)
{
	pifLog_Printf(LT_enError, "ProtocolError DC=%d", usPifId);
}

static void _evtPushSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer)
{
	uint8_t index = usPifId - PIF_ID_SWITCH;

	(void)pvIssuer;

	if (usLevel) {
		s_stProtocolTest[index].ucDataCount = rand() % 8;
		for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) s_stProtocolTest[index].ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(s_pstProtocol, &stProtocolRequests1[index], s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
			pifLog_Printf(LT_enError, "PSC(%d): DC=%d E=%d", index, s_pstProtocol->_usPifId, pif_enError);
		}
		else {
			pifLog_Printf(LT_enInfo, "PSC(%d): DC=%d CNT=%u", index, s_pstProtocol->_usPifId, s_stProtocolTest[index].ucDataCount);
#ifdef PRINT_PACKET_DATA
			if (s_stProtocolTest[index].ucDataCount) {
				pifLog_Printf(LT_enNone, "\nData:");
				for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
					pifLog_Printf(LT_enNone, " %u", s_stProtocolTest[index].ucData[i]);
				}
			}
#endif
		}
	}
}

BOOL exSerial1_Setup()
{
	int i;

    for (i = 0; i < SWITCH_COUNT; i++) {
    	s_stProtocolTest[i].pstPushSwitch = pifSensorSwitch_Add(PIF_ID_SWITCH + i, 0);
		if (!s_stProtocolTest[i].pstPushSwitch) return FALSE;
	    if (!pifSensorSwitch_AttachTask(s_stProtocolTest[i].pstPushSwitch, TM_enPeriodMs, 10, TRUE)) return FALSE;	// 10ms
		pifSensor_AttachAction(s_stProtocolTest[i].pstPushSwitch, actPushSwitchAcquire);
		pifSensor_AttachEvtChange(s_stProtocolTest[i].pstPushSwitch, _evtPushSwitchChange, NULL);
	    if (!pifSensorSwitch_AttachFilter(s_stProtocolTest[i].pstPushSwitch, PIF_SENSOR_SWITCH_FILTER_COUNT, 7, &s_stProtocolTest[i].stPushSwitchFilter)) return FALSE;
    }

    g_pstSerial1 = pifComm_Init(PIF_ID_AUTO);
	if (!g_pstSerial1) return FALSE;
    if (!pifComm_AttachTask(g_pstSerial1, TM_enPeriodMs, 1, TRUE)) return FALSE;									// 1ms
	pifComm_AttachActReceiveData(g_pstSerial1, actSerial1ReceiveData);
	pifComm_AttachActSendData(g_pstSerial1, actSerial1SendData);

    s_pstProtocol = pifProtocol_Init(PIF_ID_AUTO, g_pstTimer1ms, PT_enSmall, stProtocolQuestions1);
    if (!s_pstProtocol) return FALSE;
    pifProtocol_AttachComm(s_pstProtocol, g_pstSerial1);
    s_pstProtocol->evtError = _evtProtocolError;

    return TRUE;
}
