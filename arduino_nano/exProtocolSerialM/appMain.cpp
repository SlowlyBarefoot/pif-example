#include "appMain.h"
#include "exProtocolSerialM.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifProtocol.h"


#define COMM_COUNT         		1
#define LED_COUNT         		1
#define PROTOCOL_COUNT          1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define TASK_COUNT              2


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions[] = {
		{ 0x30, PF_enDefault, _fnProtocolQuestion30 },
		{ 0x31, PF_enDefault, _fnProtocolQuestion31 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequestTable[] = {
		{ 0x20, PF_enResponse_Yes, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_enResponse_Ack, _fnProtocolResponse21, 3, 300 },
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


static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket)
{
	s_stProtocolTest[0].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	if (pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].enFlags, NULL, 0)) {
		pifPulse_StartItem(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket)
{
	s_stProtocolTest[1].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	pifPulse_StartItem(s_stProtocolTest[1].pstDelay, 500);
}

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket)
{
	(void)pstPacket;
}

static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket)
{
	(void)pstPacket;
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
	}
	else {
		if (s_stProtocolTest[index].ucDataCount) {
			for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
			}
		}
	}
}

void appSetup()
{
	PIF_stLed *pstLedL = NULL;

    pif_Init(NULL);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
		if (!s_stProtocolTest[i].pstDelay) return;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) return;
	pifComm_AttachActReceiveData(s_pstSerial, actSerialReceiveData);
	pifComm_AttachActSendData(s_pstSerial, actSerialSendData);

    if (!pifProtocol_Init(g_pstTimer1ms, PROTOCOL_COUNT)) return;
    s_pstProtocol = pifProtocol_Add(PIF_ID_AUTO, PT_enMedium, stProtocolQuestions);
    if (!s_pstProtocol) return;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;		// 1ms
}