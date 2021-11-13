#include "appMain.h"
#include "exProtocolSerialS.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_protocol.h"


PifPulse *g_pstTimer1ms = NULL;

static PifComm *s_pstSerial = NULL;
static PifProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion30(PifProtocolPacket *pstPacket);
static void _fnProtocolQuestion31(PifProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PifProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PifProtocolPacket *pstPacket);

const PifProtocolQuestion stProtocolQuestions[] = {
		{ 0x30, PF_ANSWER_YES | PF_DEFAULT, _fnProtocolQuestion30 },
		{ 0x31, PF_ANSWER_NO | PF_DEFAULT, _fnProtocolQuestion31 },
		{ 0, PF_DEFAULT, NULL }
};

const PifProtocolRequest stProtocolRequestTable[] = {
		{ 0x20, PF_RESPONSE_YES, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_RESPONSE_NO, _fnProtocolResponse21, 3, 300 },
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


static void _fnProtocolQuestion30(PifProtocolPacket *pstPacket)
{
	s_stProtocolTest[0].ucDataCount = pstPacket->data_count;
	if (pstPacket->data_count) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->p_data, pstPacket->data_count);
	}

	if (pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].flags, NULL, 0)) {
		pifPulse_StartItem(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolQuestion31(PifProtocolPacket *pstPacket)
{
	s_stProtocolTest[1].ucDataCount = pstPacket->data_count;
	if (pstPacket->data_count) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->p_data, pstPacket->data_count);
	}

	pifPulse_StartItem(s_stProtocolTest[1].pstDelay, 500);
}

static void _fnProtocolResponse20(PifProtocolPacket *pstPacket)
{
	(void)pstPacket;
}

static void _fnProtocolResponse21(PifProtocolPacket *pstPacket)
{
	(void)pstPacket;
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
	PifLed *pstLedL;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000, 3);					// 1000us
    if (!g_pstTimer1ms) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
		if (!s_stProtocolTest[i].pstDelay) return;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

    s_pstSerial = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstSerial) return;
    if (!pifComm_AttachTask(s_pstSerial, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	s_pstSerial->act_receive_data = actSerialReceiveData;
	s_pstSerial->act_send_data = actSerialSendData;

    s_pstProtocol = pifProtocol_Create(PIF_ID_AUTO, g_pstTimer1ms, PT_SMALL, stProtocolQuestions);
    if (!s_pstProtocol) return;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);

    pifLed_BlinkOn(pstLedL, 0);
}
