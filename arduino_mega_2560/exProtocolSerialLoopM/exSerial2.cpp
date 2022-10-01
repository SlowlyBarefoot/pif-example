#include "exProtocolSerialLoopM.h"
#include "appMain.h"

#include "pif_log.h"
#include "pif_protocol.h"


PifComm g_serial2;

static PifProtocol s_protocol;

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
	PifTimer *pstDelay;
	uint8_t ucDataCount;
	uint8_t ucData[8];
} s_stProtocolTest[2] = {
		{ NULL, 0, { 0, } },
		{ NULL, 0, { 0, } }
};


static void _fnProtocolPrint(PifProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_INFO, "%s: PID=%d CNT=%u", pcName, pstPacket->packet_id, pstPacket->data_count);
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

	if (!pifProtocol_MakeAnswer(&s_protocol, pstPacket, stProtocolQuestions2[0].flags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Answer30: PID=%d Error=%d", pstPacket->packet_id, pif_error);
	}
	else {
		pifTimer_Start(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolAnswer31(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Answer31");
	s_stProtocolTest[1].ucDataCount = pstPacket->data_count;
	if (pstPacket->data_count) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->p_data, pstPacket->data_count);
	}

	pifTimer_Start(s_stProtocolTest[1].pstDelay, 500);
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

	if (!pifProtocol_MakeRequest(&s_protocol, pstOwner, s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
		pifLog_Printf(LT_ERROR, "Delay(%u): DC=%u E=%u", index, s_protocol._id, pif_error);
	}
	else {
		pifLog_Printf(LT_INFO, "Delay(%u): DC=%u CNT=%u", index, s_protocol._id, s_stProtocolTest[index].ucDataCount);
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
	if (!pifComm_Init(&g_serial2, PIF_ID_AUTO)) return FALSE;
    if (!pifComm_AttachTask(&g_serial2, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms
#ifdef USE_SERIAL
    g_serial2.act_receive_data = actSerial2ReceiveData;
    g_serial2.act_send_data = actSerial2SendData;
#endif
#ifdef USE_USART
	if (!pifComm_AllocRxBuffer(&g_serial2, 64)) return FALSE;
	if (!pifComm_AllocTxBuffer(&g_serial2, 64)) return FALSE;
	g_serial2.act_start_transfer = actUart2StartTransfer;
#endif

    if (!pifProtocol_Init(&s_protocol, PIF_ID_AUTO, &g_timer_1ms, PT_MEDIUM, stProtocolQuestions2)) return FALSE;
    pifProtocol_AttachComm(&s_protocol, &g_serial2);
    s_protocol.evt_error = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
		if (!s_stProtocolTest[i].pstDelay) return FALSE;
		pifTimer_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequest2[i]);
    }

    return TRUE;
}
