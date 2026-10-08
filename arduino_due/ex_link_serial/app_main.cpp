#include "app_main.h"

#include "protocol/pif_link.h"


PifLed g_led_l;
PifUart g_serial;
PifTimerManager g_timer_1ms;

LinkTest g_link_test[SWITCH_COUNT];

static PifLink s_link;

static void _fnLinkQuestion20(PifLink *p_owner, PifLinkPacket *p_packet);
static void _fnLinkQuestion21(PifLink *p_owner, PifLinkPacket *p_packet);

static void _fnLinkResponse30(PifLink *p_owner, PifLinkPacket *p_packet);

const PifLinkQuestion c_link_question[] = {
		{ 0x20, LINK_F_ANSWER_YES | LINK_F_LOG_PRINT_YES, _fnLinkQuestion20 },
		{ 0x21, LINK_F_ANSWER_NO | LINK_F_LOG_PRINT_YES, _fnLinkQuestion21 },
		{ 0, LINK_F_DEFAULT, NULL }
};

const PifLinkRequest c_link_requests[] = {
		{ 0x30, LINK_F_RESPONSE_YES | LINK_F_LOG_PRINT_YES, 3, 300, _fnLinkResponse30 },
		{ 0x31, LINK_F_RESPONSE_NO | LINK_F_LOG_PRINT_YES, 0, 0, NULL },
		{ 0, LINK_F_DEFAULT, 0, 0, NULL }
};


static void _fnLinkPrint(PifLinkPacket *p_packet, const char *p_name)
{
	if (p_packet) {
		pifLog_Printf(LT_INFO, "%s: PID=%d CNT=%u", p_name, p_packet->packet_id, p_packet->data_count);
		if (p_packet->data_count) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (uint16_t i = 0; i < p_packet->data_count; i++) {
				pifLog_Printf(LT_NONE, " %u", p_packet->p_data[i]);
			}
		}
	}
	else {
		pifLog_Printf(LT_INFO, "%s", p_name);
	}
}

static void _fnCompareData(PifLinkPacket *p_packet, uint8_t index)
{
	uint16_t i;

	if (p_packet->data_count == g_link_test[index].data_count) {
		for (i = 0; i < p_packet->data_count; i++) {
			if (p_packet->p_data[i] != g_link_test[index].data[i]) break;
		}
		if (i < p_packet->data_count) {
			pifLog_Printf(LT_INFO, "Different data");
		}
		else {
			pifLog_Printf(LT_INFO, "Same data");
		}
	}
	else {
		pifLog_Printf(LT_ERROR, "Different count: %u != %u", g_link_test[index].data_count, p_packet->data_count);
	}
}

static void _fnLinkQuestion20(PifLink *p_owner, PifLinkPacket *p_packet)
{
	_fnCompareData(p_packet, 0);
	_fnLinkPrint(p_packet, "Question20");

	if (!pifLink_MakeAnswer(p_owner, p_packet, LINK_F_LOG_PRINT_YES, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Question20: PID=%d Error=%d", p_packet->packet_id, pif_error);
	}
}

static void _fnLinkQuestion21(PifLink *p_owner, PifLinkPacket *p_packet)
{
	(void)p_owner;

	_fnCompareData(p_packet, 1);
	_fnLinkPrint(p_packet, "Question21");
}

static void _fnLinkResponse30(PifLink *p_owner, PifLinkPacket *p_packet)
{
	(void)p_owner;

	_fnLinkPrint(p_packet, "Response30");
}

static void _evtLinkError(PifLink *p_owner, const PifLinkRequest *p_request, PifLinkError error)
{
	(void)p_request;

	pifLog_Printf(LT_ERROR, "LinkError DC=%d Error=%d", p_owner->_id, error);
}

static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	uint8_t index = p_owner->_id - PIF_ID_SWITCH;

	(void)p_value;
	(void)p_issuer;

	if (state) {
		g_link_test[index].data_count = rand() % 8;
		for (int i = 0; i < g_link_test[index].data_count; i++) g_link_test[index].data[i] = rand() & 0xFF;
		if (!pifLink_MakeRequest(&s_link, 0, &c_link_requests[index], g_link_test[index].data, g_link_test[index].data_count)) {
			pifLog_Printf(LT_ERROR, "PushSwitchChange(%d): DC=%d E=%d", index, s_link._id, pif_error);
		}
		else {
			pifLog_Printf(LT_INFO, "PushSwitchChange(%d): DC=%d CNT=%u", index, s_link._id, g_link_test[index].data_count);
			if (g_link_test[index].data_count) {
				pifLog_Printf(LT_NONE, "\nData:");
				for (int i = 0; i < g_link_test[index].data_count; i++) {
					pifLog_Printf(LT_NONE, " %u", g_link_test[index].data[i]);
				}
			}
		}
	}
}

BOOL appSetup()
{
	int i;
	static PifNoiseFilterManager push_switch_filter;

    if (!pifNoiseFilterManager_Init(&push_switch_filter, SWITCH_COUNT)) return FALSE;
    for (i = 0; i < SWITCH_COUNT; i++) {
	    if (!pifSensorSwitch_AttachTaskAcquire(&g_link_test[i].push_switch, PIF_ID_AUTO, TM_PERIOD, 10000, TRUE)) return FALSE;	// 10ms
		pifSensor_AttachEvtChange(&g_link_test[i].push_switch.parent, _evtPushSwitchChange, NULL);
		g_link_test[i].push_switch.p_filter = pifNoiseFilterBit_AddCount(&push_switch_filter, 7);
	    if (!g_link_test[i].push_switch.p_filter) return FALSE;
    	g_link_test[i].data_count = 0;
    }

    if (!pifLink_Init(&s_link, PIF_ID_AUTO, &g_timer_1ms, LINK_T_SINGLE, 0, c_link_question)) return FALSE;
    pifLink_AttachUart(&s_link, &g_serial);
    s_link.evt_error = _evtLinkError;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;																	// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
