#include <string.h>

#include "appMain.h"
#include "main.h"

#include "core/pif_log.h"
#include "protocol/pif_protocol.h"


PifTimerManager g_timer_1ms;

static PifComm s_comm_log;
static PifComm s_serial;
static PifProtocol s_protocol;

static void _fnProtocolQuestion30(PifProtocolPacket *pstPacket);
static void _fnProtocolQuestion31(PifProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PifProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PifProtocolPacket *pstPacket);

const PifProtocolQuestion stProtocolQuestions[] = {
		{ 0x30, PF_ANSWER_YES | PF_LOG_PRINT_YES, _fnProtocolQuestion30 },
		{ 0x31, PF_ANSWER_NO | PF_LOG_PRINT_YES, _fnProtocolQuestion31 },
		{ 0, PF_DEFAULT, NULL }
};

const PifProtocolRequest stProtocolRequestTable[] = {
		{ 0x20, PF_RESPONSE_YES | PF_LOG_PRINT_YES, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_RESPONSE_NO | PF_LOG_PRINT_YES, _fnProtocolResponse21, 3, 300 },
		{ 0, PF_DEFAULT, NULL, 0, 0 }
};

static struct {
	PifTimer *pstDelay;
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

static void _fnProtocolQuestion30(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question30");
	s_stProtocolTest[0].ucDataCount = pstPacket->data_count;
	if (pstPacket->data_count) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->p_data, pstPacket->data_count);
	}

	if (!pifProtocol_MakeAnswer(&s_protocol, pstPacket, stProtocolQuestions[0].flags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Question30: Error=%d", pif_error);
	}
	else {
		pifTimer_Start(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolQuestion31(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question31");
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

	pifLog_Printf(LT_INFO, "Response21: ACK");
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
		pifLog_Printf(LT_ERROR, "Delay(%u): DC=%d E=%d", index, s_protocol._id, pif_error);
	}
	else {
		pifLog_Printf(LT_INFO, "Delay(%u): DC=%d CNT=%u", index, s_protocol._id, s_stProtocolTest[index].ucDataCount);
		if (s_stProtocolTest[index].ucDataCount) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
				pifLog_Printf(LT_NONE, " %u", s_stProtocolTest[index].ucData[i]);
			}
		}
	}
}

BOOL appInit()
{
    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return FALSE;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 4)) return FALSE;	// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return FALSE;
	s_comm_log.act_send_data = actLogSendData;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return FALSE;		// 1ms

	if (!pifLog_AttachComm(&s_comm_log)) return FALSE;

	if (!pifComm_Init(&s_serial, PIF_ID_AUTO)) return FALSE;
	s_serial.act_receive_data = actSerialReceiveData;
	s_serial.act_send_data = actSerialSendData;
    if (!pifComm_AttachTask(&s_serial, TM_PERIOD_MS, 1, TRUE)) return FALSE;		// 1ms

    if (!pifProtocol_Init(&s_protocol, PIF_ID_AUTO, &g_timer_1ms, PT_SMALL, stProtocolQuestions)) return FALSE;
    pifProtocol_AttachComm(&s_protocol, &s_serial);
    s_protocol.evt_error = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
		if (!s_stProtocolTest[i].pstDelay) return FALSE;
		pifTimer_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
    return TRUE;
}

void appExit()
{
	pifProtocol_Clear(&s_protocol);
	pifTimerManager_Clear(&g_timer_1ms);
	pifComm_Clear(&s_serial);
	pifComm_Clear(&s_comm_log);
    pifLog_Clear();
    pif_Exit();
}
