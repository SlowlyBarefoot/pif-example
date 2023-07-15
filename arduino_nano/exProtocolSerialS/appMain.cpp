#include "appMain.h"
#include "exProtocolSerialS.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "protocol/pif_protocol.h"


PifTimerManager g_timer_1ms;

static PifUart s_serial;
static PifProtocol s_protocol;

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
	PifTimer *pstDelay;
	uint8_t ucDataCount;
	uint8_t ucData[8];
} s_stProtocolTest[SWITCH_COUNT] = {
		{ NULL, 0, { 0, } },
		{ NULL, 0, { 0, } }
};


static void _fnProtocolQuestion30(PifProtocolPacket *pstPacket)
{
	s_stProtocolTest[0].ucDataCount = pstPacket->data_count;
	if (pstPacket->data_count) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->p_data, pstPacket->data_count);
	}

	if (pifProtocol_MakeAnswer(&s_protocol, pstPacket, stProtocolQuestions[0].flags, NULL, 0)) {
		pifTimer_Start(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolQuestion31(PifProtocolPacket *pstPacket)
{
	s_stProtocolTest[1].ucDataCount = pstPacket->data_count;
	if (pstPacket->data_count) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->p_data, pstPacket->data_count);
	}

	pifTimer_Start(s_stProtocolTest[1].pstDelay, 500);
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

	if (!pifProtocol_MakeRequest(&s_protocol, pstOwner, s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
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
	int i;
	static PifLed s_led_l;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;		// 1000us

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;							// 500ms

    for (i = 0; i < SWITCH_COUNT; i++) {
    	s_stProtocolTest[i].pstDelay = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
		if (!s_stProtocolTest[i].pstDelay) return;
		pifTimer_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

	if (!pifUart_Init(&s_serial, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_serial, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
    s_serial.act_receive_data = actSerialReceiveData;
    s_serial.act_send_data = actSerialSendData;

    if (!pifProtocol_Init(&s_protocol, PIF_ID_AUTO, &g_timer_1ms, PT_SMALL, stProtocolQuestions)) return;
    pifProtocol_AttachUart(&s_protocol, &s_serial);

    pifLed_SBlinkOn(&s_led_l, 1 << 0);
}
