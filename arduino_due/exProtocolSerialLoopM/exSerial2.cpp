#include "exProtocolSerialLoopM.h"
#include "appMain.h"

#include "pifLog.h"
#include "pifProtocol.h"


PifComm *g_pstSerial2 = NULL;

static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolAnswer30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolAnswer31(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions2[] = {
		{ 0x30, PF_enAnswer_Yes | PF_enLogPrint_Yes, _fnProtocolAnswer30 },
		{ 0x31, PF_enAnswer_No | PF_enLogPrint_Yes, _fnProtocolAnswer31 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequest2[] = {
		{ 0x20, PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_enResponse_No | PF_enLogPrint_Yes, _fnProtocolResponse21, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};

static struct {
	PifPulseItem *pstDelay;
	uint8_t ucDataCount;
	uint8_t ucData[8];
} s_stProtocolTest[2] = {
		{ NULL, 0, },
		{ NULL, 0, }
};


static void _fnProtocolPrint(PIF_stProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_INFO, "%s: PID=%d CNT=%u", pcName, pstPacket->ucPacketId, pstPacket->usDataCount);
#ifdef PRINT_PACKET_DATA
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (int i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_NONE, " %u", pstPacket->pucData[i]);
			}
		}
#endif
	}
	else {
		pifLog_Printf(LT_INFO, pcName);
	}
}

static void _fnProtocolAnswer30(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Answer30");
	s_stProtocolTest[0].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions2[0].enFlags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Answer30: PID=%d Error=%d", pstPacket->ucPacketId, pif_error);
	}
	else {
		pifPulse_StartItem(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolAnswer31(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Answer31");
	s_stProtocolTest[1].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	pifPulse_StartItem(s_stProtocolTest[1].pstDelay, 500);
}

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response20");
}

static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_INFO, "Response21");
}

static void _evtProtocolError(PifId usPifId)
{
	pifLog_Printf(LT_ERROR, "eventProtocolError DC=%d", usPifId);
}

static void _evtDelay(void *pvIssuer)
{
	if (!pvIssuer) {
		pif_error = E_INVALID_PARAM;
		return;
	}

	const PIF_stProtocolRequest *pstOwner = (PIF_stProtocolRequest *)pvIssuer;
	int index = pstOwner->ucCommand & 0x0F;

	if (!pifProtocol_MakeRequest(s_pstProtocol, pstOwner, s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
		pifLog_Printf(LT_ERROR, "Delay(%u): DC=%u E=%u", index, s_pstProtocol->_usPifId, pif_error);
	}
	else {
		pifLog_Printf(LT_INFO, "Delay(%u): DC=%u CNT=%u", index, s_pstProtocol->_usPifId, s_stProtocolTest[index].ucDataCount);
#ifdef PRINT_PACKET_DATA
		if (s_stProtocolTest[index].ucDataCount) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
				pifLog_Printf(LT_NONE, " %u", s_stProtocolTest[index].ucData[i]);
			}
		}
#endif
	}
}

BOOL exSerial2_Setup()
{
	g_pstSerial2 = pifComm_Create(PIF_ID_AUTO);
	if (!g_pstSerial2) return FALSE;
    if (!pifComm_AttachTask(g_pstSerial2, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms
	pifComm_AttachActReceiveData(g_pstSerial2, actSerial2ReceiveData);
	pifComm_AttachActSendData(g_pstSerial2, actSerial2SendData);

    s_pstProtocol = pifProtocol_Create(PIF_ID_AUTO, g_pstTimer1ms, PT_enMedium, stProtocolQuestions2);
    if (!s_pstProtocol) return FALSE;
    pifProtocol_AttachComm(s_pstProtocol, g_pstSerial2);
    s_pstProtocol->evtError = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
		if (!s_stProtocolTest[i].pstDelay) return FALSE;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequest2[i]);
    }

    return TRUE;
}
