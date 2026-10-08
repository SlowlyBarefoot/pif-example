#include "app_main.h"


PifUart g_serial2;

static PifLink s_link;

static void _fnLinkAnswer30(PifLink *p_owner, PifLinkPacket *p_packet);
static void _fnLinkAnswer31(PifLink *p_owner, PifLinkPacket *p_packet);

static void _fnLinkResponse20(PifLink *p_owner, PifLinkPacket *p_packet);

const PifLinkQuestion c_link_question2[] = {
		{ 0x30, LINK_F_ANSWER_YES | LINK_F_LOG_PRINT_YES, _fnLinkAnswer30 },
		{ 0x31, LINK_F_ANSWER_NO | LINK_F_LOG_PRINT_YES, _fnLinkAnswer31 },
		{ 0, LINK_F_DEFAULT, NULL }
};

const PifLinkRequest c_link_request2[] = {
		{ 0x20, LINK_F_RESPONSE_YES | LINK_F_LOG_PRINT_YES, 3, 300, _fnLinkResponse20 },
		{ 0x21, LINK_F_RESPONSE_NO | LINK_F_LOG_PRINT_YES, 0, 0, NULL },
		{ 0, LINK_F_DEFAULT, 0, 0, NULL }
};

static struct {
	PifTimer *p_delay;
	uint16_t data_count;
	uint8_t data[PIF_LINK_RX_PACKET_SIZE];
} s_link_test[2] = {
		{ NULL, 0, { 0, } },
		{ NULL, 0, { 0, } }
};


static void _fnLinkPrint(PifLinkPacket *p_packet, const char *p_name)
{
	if (p_packet) {
		pifLog_Printf(LT_INFO, "%s: PID=%d CNT=%u", p_name, p_packet->packet_id, p_packet->data_count);
#ifdef PRINT_PACKET_DATA
		if (p_packet->data_count) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (int i = 0; i < p_packet->data_count; i++) {
				pifLog_Printf(LT_NONE, " %u", p_packet->p_data[i]);
			}
		}
#endif
	}
	else {
		pifLog_Printf(LT_INFO, "%s", p_name);
	}
}

static void _fnLinkAnswer30(PifLink *p_owner, PifLinkPacket *p_packet)
{
	_fnLinkPrint(p_packet, "Answer30");
	s_link_test[0].data_count = p_packet->data_count;
	if (p_packet->data_count) {
		memcpy(s_link_test[0].data, p_packet->p_data, p_packet->data_count);
	}

	if (!pifLink_MakeAnswer(p_owner, p_packet, LINK_F_LOG_PRINT_YES, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Answer30: PID=%d Error=%d", p_packet->packet_id, pif_error);
	}
	else {
		pifTimer_Start(s_link_test[0].p_delay, 500);
	}
}

static void _fnLinkAnswer31(PifLink *p_owner, PifLinkPacket *p_packet)
{
	(void)p_owner;

	_fnLinkPrint(p_packet, "Answer31");
	s_link_test[1].data_count = p_packet->data_count;
	if (p_packet->data_count) {
		memcpy(s_link_test[1].data, p_packet->p_data, p_packet->data_count);
	}

	pifTimer_Start(s_link_test[1].p_delay, 500);
}

static void _fnLinkResponse20(PifLink *p_owner, PifLinkPacket *p_packet)
{
	(void)p_owner;

	_fnLinkPrint(p_packet, "Response20");
}

static void _evtLinkError(PifLink *p_owner, const PifLinkRequest *p_request, PifLinkError error)
{
	(void)p_request;

	pifLog_Printf(LT_ERROR, "eventLinkError DC=%d Error=%d", p_owner->_id, error);
}

static void _evtDelay(void *p_issuer)
{
	if (!p_issuer) {
		pif_error = E_INVALID_PARAM;
		return;
	}

	const PifLinkRequest *p_owner = (PifLinkRequest *)p_issuer;
	int index = p_owner->command & 0x0F;

	if (!pifLink_MakeRequest(&s_link, 0, p_owner, s_link_test[index].data, s_link_test[index].data_count)) {
		pifLog_Printf(LT_ERROR, "Delay(%u): DC=%u E=%u", index, s_link._id, pif_error);
	}
	else {
		pifLog_Printf(LT_INFO, "Delay(%u): DC=%u CNT=%u", index, s_link._id, s_link_test[index].data_count);
#ifdef PRINT_PACKET_DATA
		if (s_link_test[index].data_count) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (int i = 0; i < s_link_test[index].data_count; i++) {
				pifLog_Printf(LT_NONE, " %u", s_link_test[index].data[i]);
			}
		}
#endif
	}
}

BOOL exSerial2_Setup()
{
    if (!pifLink_Init(&s_link, PIF_ID_AUTO, &g_timer_1ms, LINK_T_SINGLE, 0, c_link_question2)) return FALSE;
    pifLink_AttachUart(&s_link, &g_serial2);
    s_link.evt_error = _evtLinkError;

    for (int i = 0; i < 2; i++) {
    	s_link_test[i].p_delay = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
		if (!s_link_test[i].p_delay) return FALSE;
		pifTimer_AttachEvtFinish(s_link_test[i].p_delay, _evtDelay, (void *)&c_link_request2[i]);
    }

    return TRUE;
}
