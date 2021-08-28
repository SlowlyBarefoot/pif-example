#include <string.h>

#include "appMain.h"
#include "main.h"

#include "pifLog.h"
#include "pifProtocol.h"


#define COMM_COUNT         		2
#define PROTOCOL_COUNT          1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              3


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions[] = {
		{ 0x30, PF_enAnswer_Yes | PF_enLogPrint_Yes, _fnProtocolQuestion30 },
		{ 0x31, PF_enAnswer_No | PF_enLogPrint_Yes, _fnProtocolQuestion31 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequestTable[] = {
		{ 0x20, PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_enResponse_No | PF_enLogPrint_Yes, _fnProtocolResponse21, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};

static struct {
	PIF_stPulseItem *pstDelay;
	uint8_t ucDataCount;
	uint8_t ucData[8];
} s_stProtocolTest[2] = {
		{ NULL, 0, },
		{ NULL, 0, }
};


static void _fnProtocolPrint(PIF_stProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_enInfo, "%s: PID=%d CNT=%u", pcName, pstPacket->ucPacketId, pstPacket->usDataCount);
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (uint16_t i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
		}
	}
	else {
		pifLog_Printf(LT_enInfo, pcName);
	}
}

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question30");
	s_stProtocolTest[0].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].enFlags, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "Question30: PID=%d Error=%d", pstPacket->ucPacketId, pif_enError);
	}
	else {
		pifPulse_StartItem(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question31");
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

	pifLog_Printf(LT_enInfo, "Response21: ACK");
}

static void _evtProtocolError(PIF_usId usPifId)
{
	pifLog_Printf(LT_enError, "eventProtocolError DC=%d", usPifId);
}

static void _evtDelay(void *pvIssuer)
{
	if (!pvIssuer) {
		pif_enError = E_enInvalidParam;
		return;
	}

	const PIF_stProtocolRequest *pstOwner = (PIF_stProtocolRequest *)pvIssuer;
	int index = pstOwner->ucCommand & 0x0F;

	if (!pifProtocol_MakeRequest(s_pstProtocol, pstOwner, s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
		pifLog_Printf(LT_enError, "Delay(%u): DC=%d E=%d", index, s_pstProtocol->_usPifId, pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "Delay(%u): DC=%d CNT=%u", index, s_pstProtocol->_usPifId, s_stProtocolTest[index].ucDataCount);
		if (s_stProtocolTest[index].ucDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", s_stProtocolTest[index].ucData[i]);
			}
		}
	}
}

BOOL appInit()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return FALSE;
    if (!pifPulse_Init(PULSE_COUNT)) return FALSE;
    if (!pifTask_Init(TASK_COUNT)) return FALSE;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return FALSE;

    if (!pifProtocol_Init(PROTOCOL_COUNT, g_pstTimer1ms)) return FALSE;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return FALSE;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return FALSE;

    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) return FALSE;
	pifComm_AttachActReceiveData(s_pstSerial, actSerialReceiveData);
	pifComm_AttachActSendData(s_pstSerial, actSerialSendData);

    s_pstProtocol = pifProtocol_Add(PIF_ID_AUTO, PT_enMedium, stProtocolQuestions);
    if (!s_pstProtocol) return FALSE;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    s_pstProtocol->evtError = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
		if (!s_stProtocolTest[i].pstDelay) return FALSE;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return FALSE;		// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return FALSE;		// 1ms
    return TRUE;
}

void appExit()
{
	pifTask_Exit();
	pifProtocol_Exit();
	pifPulse_Exit();
	pifComm_Exit();
    pifLog_Exit();
}
