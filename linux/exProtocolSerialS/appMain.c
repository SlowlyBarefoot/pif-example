#include <string.h>

#include "appMain.h"
#include "main.h"

#include "pif_log.h"
#include "pif_protocol.h"


PifPulse *g_pstTimer1ms = NULL;

static PifComm *s_pstCommLog = NULL;
static PifComm *s_pstSerial = NULL;
static PifProtocol *s_pstProtocol = NULL;

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

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].flags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Question30: Error=%d", pif_error);
	}
	else {
		pifPulse_StartItem(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolQuestion31(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question31");
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

	if (!pifProtocol_MakeRequest(s_pstProtocol, pstOwner, s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
		pifLog_Printf(LT_ERROR, "Delay(%u): DC=%d E=%d", index, s_pstProtocol->_id, pif_error);
	}
	else {
		pifLog_Printf(LT_INFO, "Delay(%u): DC=%d CNT=%u", index, s_pstProtocol->_id, s_stProtocolTest[index].ucDataCount);
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

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000, 4);						// 1000us
    if (!g_pstTimer1ms) return FALSE;

    s_pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstCommLog) return FALSE;
	s_pstCommLog->act_send_data = actLogSendData;
    if (!pifComm_AttachTask(s_pstCommLog, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms

	if (!pifLog_AttachComm(s_pstCommLog)) return FALSE;

    s_pstSerial = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstSerial) return FALSE;
	s_pstSerial->act_receive_data = actSerialReceiveData;
	s_pstSerial->act_send_data = actSerialSendData;
    if (!pifComm_AttachTask(s_pstSerial, TM_PERIOD_MS, 1, TRUE)) return FALSE;	// 1ms

    s_pstProtocol = pifProtocol_Create(PIF_ID_AUTO, g_pstTimer1ms, PT_SMALL, stProtocolQuestions);
    if (!s_pstProtocol) return FALSE;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    s_pstProtocol->evt_error = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
		if (!s_stProtocolTest[i].pstDelay) return FALSE;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifPulse_Count(g_pstTimer1ms));
    return TRUE;
}

void appExit()
{
	pifProtocol_Destroy(&s_pstProtocol);
	pifPulse_Destroy(&g_pstTimer1ms);
	pifComm_Destroy(&s_pstSerial);
	pifComm_Destroy(&s_pstCommLog);
    pifLog_Clear();
    pif_Exit();
}
