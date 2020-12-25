#include "exProtocolSerialLoopM.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifProtocol.h"


static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions[] = {
		{ 0x30, PF_enLogPrint_Yes, _fnProtocolQuestion30 },
		{ 0x31, PF_enLogPrint_Yes, _fnProtocolQuestion31 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequestTable[] = {
		{ 0x20, PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_enResponse_Ack | PF_enLogPrint_Yes, _fnProtocolResponse21, 3, 300 },
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
			for (int i = 0; i < pstPacket->usDataCount; i++) {
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

static void _evtProtocolError(PIF_unDeviceCode unDeviceCode)
{
	pifLog_Printf(LT_enError, "eventProtocolError DC=%d", unDeviceCode);
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
		pifLog_Printf(LT_enError, "Delay(%u): DC=%d E=%d", index, s_pstProtocol->unDeviceCode, pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "Delay(%u): DC=%d CNT=%u", index, s_pstProtocol->unDeviceCode, s_stProtocolTest[index].ucDataCount);
		if (s_stProtocolTest[index].ucDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", s_stProtocolTest[index].ucData[i]);
			}
		}
	}
}

static void _taskProtocolTest(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    while (pifComm_SendData(s_pstSerial, &txData)) {
    	Serial2.print((char)txData);
    }

    while (pifComm_GetRemainSizeOfRxBuffer(s_pstSerial)) {
		rxData = Serial2.read();
		if (rxData >= 0) {
			pifComm_ReceiveData(s_pstSerial, rxData);
		}
		else break;
	}
}

PIF_unDeviceCode exSerial2_Setup(PIF_unDeviceCode unDeviceCode)
{
	Serial2.begin(115200);

    s_pstSerial = pifComm_Add(unDeviceCode++);
	if (!s_pstSerial) return 0;

    s_pstProtocol = pifProtocol_Add(unDeviceCode++, PT_enMedium, stProtocolQuestions);
    if (!s_pstProtocol) return 0;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    s_pstProtocol->evtError = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(g_pstTimer, PT_enOnce);
		if (!s_stProtocolTest[i].pstDelay) return 0;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

    if (!pifTask_AddRatio(3, _taskProtocolTest, NULL)) return 0;		// 3%

    return unDeviceCode;
}
