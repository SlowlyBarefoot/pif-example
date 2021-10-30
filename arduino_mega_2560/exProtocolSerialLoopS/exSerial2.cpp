#include "exProtocolSerialLoopS.h"
#include "appMain.h"

#include "pif_log.h"
#include "pif_protocol.h"


PifComm *g_pstSerial2 = NULL;

static PifProtocol *s_pstProtocol = NULL;

static void _fnProtocolAnswer30(PifProtocolPacket *pstPacket);
static void _fnProtocolAnswer31(PifProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PifProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PifProtocolPacket *pstPacket);

const PifProtocolQuestion stProtocolQuestions2[] = {
		{ 0x30, PF_ANSWER_YES | PF_LOG_PRINT_YES, _fnProtocolAnswer30 },
		{ 0x31, PF_ANSWER_NO | PF_LOG_PRINT_YES, _fnProtocolAnswer31 },
		{ 0, PF_DEFAULT, NULL }
};

const PifProtocolRequest stProtocolRequest2[] = {
		{ 0x20, PF_RESPONSE_YES | PF_LOG_PRINT_YES, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_RESPONSE_NO | PF_LOG_PRINT_YES, _fnProtocolResponse21, 3, 300 },
		{ 0, PF_DEFAULT, NULL, 0, 0 }
};

static struct {
	PifPulseItem *pstDelay;
	uint8_t ucDataCount;
	uint8_t ucData[8];
} s_stProtocolTest[2] = {
		{ NULL, 0, },
		{ NULL, 0, }
};


static void _fnProtocolPrint(PifProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_INFO, "%s: CNT=%u", pcName, pstPacket->data_count);
#ifdef PRINT_PACKET_DATA
		if (pstPacket->data_count) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (int i = 0; i < pstPacket->data_count; i++) {
				pifLog_Printf(LT_NONE, " %u", pstPacket->p_data[i]);
			}
		}
#endif
	}
	else {
		pifLog_Printf(LT_INFO, pcName);
	}
}

static void _fnProtocolAnswer30(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Answer30");
	s_stProtocolTest[0].ucDataCount = pstPacket->data_count;
	if (pstPacket->data_count) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->p_data, pstPacket->data_count);
	}

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions2[0].flags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Answer30: Error=%d", pif_error);
	}
	else {
		pifPulse_StartItem(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolAnswer31(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Answer31");
	s_stProtocolTest[1].ucDataCount = pstPacket->data_count;
	if (pstPacket->data_count) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->p_data, pstPacket->data_count);
	}

	pifPulse_StartItem(s_stProtocolTest[1].pstDelay, 500);
}

static void _fnProtocolResponse20(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response20");
}

static void _fnProtocolResponse21(PifProtocolPacket *pstPacket)
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

	const PifProtocolRequest *pstOwner = (PifProtocolRequest *)pvIssuer;
	int index = pstOwner->command & 0x0F;

	if (!pifProtocol_MakeRequest(s_pstProtocol, pstOwner, s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
		pifLog_Printf(LT_ERROR, "Delay(%u): DC=%u E=%u", index, s_pstProtocol->_id, pif_error);
	}
	else {
		pifLog_Printf(LT_INFO, "Delay(%u): DC=%u CNT=%u", index, s_pstProtocol->_id, s_stProtocolTest[index].ucDataCount);
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
#ifdef USE_SERIAL
	g_pstSerial2->act_receive_data = actSerial2ReceiveData;
	g_pstSerial2->act_send_data = actSerial2SendData;
#endif
#ifdef USE_USART
	if (!pifComm_AllocRxBuffer(g_pstSerial2, 64)) return FALSE;
	if (!pifComm_AllocTxBuffer(g_pstSerial2, 64)) return FALSE;
	g_pstSerial2->act_start_transfer = actUart2StartTransfer;
#endif

    s_pstProtocol = pifProtocol_Create(PIF_ID_AUTO, g_pstTimer1ms, PT_SMALL, stProtocolQuestions2);
    if (!s_pstProtocol) return FALSE;
    pifProtocol_AttachComm(s_pstProtocol, g_pstSerial2);
    s_pstProtocol->evt_error = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
		if (!s_stProtocolTest[i].pstDelay) return FALSE;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequest2[i]);
    }

    return TRUE;
}
