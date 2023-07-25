#include "appMain.h"

#include "protocol/pif_protocol.h"


PifLed g_led_l;
PifUart g_serial;
PifTimerManager g_timer_1ms;

ProtocolTest g_stProtocolTest[SWITCH_COUNT];

static PifProtocol s_protocol;

static void _fnProtocolQuestion20(PifProtocolPacket *pstPacket);
static void _fnProtocolQuestion21(PifProtocolPacket *pstPacket);

static void _fnProtocolResponse30(PifProtocolPacket *pstPacket);
static void _fnProtocolResponse31(PifProtocolPacket *pstPacket);

const PifProtocolQuestion stProtocolQuestions[] = {
		{ 0x20, PF_ANSWER_YES | PF_LOG_PRINT_YES, _fnProtocolQuestion20 },
		{ 0x21, PF_ANSWER_NO | PF_LOG_PRINT_YES, _fnProtocolQuestion21 },
		{ 0, PF_DEFAULT, NULL }
};

const PifProtocolRequest stProtocolRequests[] = {
		{ 0x30, PF_RESPONSE_YES | PF_LOG_PRINT_YES, _fnProtocolResponse30, 3, 300 },
		{ 0x31, PF_RESPONSE_NO | PF_LOG_PRINT_YES, _fnProtocolResponse31, 3, 300 },
		{ 0, PF_DEFAULT, NULL, 0, 0 }
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

static void _fnCompareData(PifProtocolPacket *pstPacket, uint8_t ucIndex)
{
	uint16_t i;

	if (pstPacket->data_count == g_stProtocolTest[ucIndex].ucDataCount) {
		for (i = 0; i < pstPacket->data_count; i++) {
			if (pstPacket->p_data[i] != g_stProtocolTest[ucIndex].ucData[i]) break;
		}
		if (i < pstPacket->data_count) {
			pifLog_Printf(LT_INFO, "Different data");
		}
		else {
			pifLog_Printf(LT_INFO, "Same data");
		}
	}
	else {
		pifLog_Printf(LT_ERROR, "Different count: %u != %u", g_stProtocolTest[ucIndex].ucDataCount, pstPacket->data_count);
	}
}

static void _fnProtocolQuestion20(PifProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 0);
	_fnProtocolPrint(pstPacket, "Question20");

	if (!pifProtocol_MakeAnswer(&s_protocol, pstPacket, stProtocolQuestions[0].flags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Question20: Error=%d", pif_error);
	}
}

static void _fnProtocolQuestion21(PifProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 1);
	_fnProtocolPrint(pstPacket, "Question21");
}

static void _fnProtocolResponse30(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response30");
}

static void _fnProtocolResponse31(PifProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_INFO, "Response31: ACK");
}

static void _evtProtocolError(PifId usPifId)
{
	pifLog_Printf(LT_ERROR, "ProtocolError DC=%d", usPifId);
}

static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	uint8_t index = p_owner->_id - PIF_ID_SWITCH;

	(void)p_value;
	(void)p_issuer;

	if (state) {
		g_stProtocolTest[index].ucDataCount = rand() % 8;
		for (int i = 0; i < g_stProtocolTest[index].ucDataCount; i++) g_stProtocolTest[index].ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(&s_protocol, &stProtocolRequests[index], g_stProtocolTest[index].ucData, g_stProtocolTest[index].ucDataCount)) {
			pifLog_Printf(LT_ERROR, "PushSwitchChange(%d): DC=%d E=%d", index, s_protocol._id, pif_error);
		}
		else {
			pifLog_Printf(LT_INFO, "PushSwitchChange(%d): DC=%d CNT=%u", index, s_protocol._id, g_stProtocolTest[index].ucDataCount);
			if (g_stProtocolTest[index].ucDataCount) {
				pifLog_Printf(LT_NONE, "\nData:");
				for (int i = 0; i < g_stProtocolTest[index].ucDataCount; i++) {
					pifLog_Printf(LT_NONE, " %u", g_stProtocolTest[index].ucData[i]);
				}
			}
		}
	}
}

BOOL appSetup()
{
	int i;

    for (i = 0; i < SWITCH_COUNT; i++) {
	    if (!pifNoiseFilterBit_Init(&g_stProtocolTest[i].stPushSwitchFilter, 7)) return FALSE;

	    if (!pifSensorSwitch_AttachTaskAcquire(&g_stProtocolTest[i].stPushSwitch, TM_PERIOD_MS, 10, TRUE)) return FALSE;	// 10ms
		g_stProtocolTest[i].stPushSwitch.parent.evt_change = _evtPushSwitchChange;
		g_stProtocolTest[i].stPushSwitch.p_filter = &g_stProtocolTest[i].stPushSwitchFilter.parent;
	    g_stProtocolTest[i].ucDataCount = 0;
    }

    if (!pifProtocol_Init(&s_protocol, PIF_ID_AUTO, &g_timer_1ms, PT_SMALL, stProtocolQuestions)) return FALSE;
    pifProtocol_AttachUart(&s_protocol, &g_serial);
    s_protocol.evt_error = _evtProtocolError;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;																	// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
