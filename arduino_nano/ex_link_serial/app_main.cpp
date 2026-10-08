#include "app_main.h"

#include "protocol/pif_link.h"


PifLed g_led_l;
PifUart g_serial;
PifTimerManager g_timer_1ms;

static PifLink s_link;

static void _fnLinkQuestion30(PifLink *p_owner, PifLinkPacket *p_packet);
static void _fnLinkQuestion31(PifLink *p_owner, PifLinkPacket *p_packet);

static void _fnLinkResponse20(PifLink *p_owner, PifLinkPacket *p_packet);

const PifLinkQuestion c_link_question[] = {
		{ 0x30, LINK_F_ANSWER_YES | LINK_F_DEFAULT, _fnLinkQuestion30 },
		{ 0x31, LINK_F_ANSWER_NO | LINK_F_DEFAULT, _fnLinkQuestion31 },
		{ 0, LINK_F_DEFAULT, NULL }
};

const PifLinkRequest c_link_request[] = {
		{ 0x20, LINK_F_RESPONSE_YES, 3, 300, _fnLinkResponse20 },
		{ 0x21, LINK_F_RESPONSE_NO, 0, 0, NULL },
		{ 0, LINK_F_DEFAULT, 0, 0, NULL }
};

static struct {
	PifTimer *p_delay;
	uint8_t data_count;
	uint8_t data[8];
} s_link_test[SWITCH_COUNT] = {
		{ NULL, 0, { 0, } },
		{ NULL, 0, { 0, } }
};


static void _fnLinkQuestion30(PifLink *p_owner, PifLinkPacket *p_packet)
{
	s_link_test[0].data_count = p_packet->data_count;
	if (p_packet->data_count) {
		memcpy(s_link_test[0].data, p_packet->p_data, p_packet->data_count);
	}

	if (pifLink_MakeAnswer(p_owner, p_packet, LINK_F_LOG_PRINT_YES, NULL, 0)) {
		pifTimer_Start(s_link_test[0].p_delay, 500);
	}
}

static void _fnLinkQuestion31(PifLink *p_owner, PifLinkPacket *p_packet)
{
	s_link_test[1].data_count = p_packet->data_count;
	if (p_packet->data_count) {
		memcpy(s_link_test[1].data, p_packet->p_data, p_packet->data_count);
	}

	pifTimer_Start(s_link_test[1].p_delay, 500);
}

static void _fnLinkResponse20(PifLink *p_owner, PifLinkPacket *p_packet)
{
	(void)p_owner;
	(void)p_packet;
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
	}
	else {
		if (s_link_test[index].data_count) {
			for (int i = 0; i < s_link_test[index].data_count; i++) {
			}
		}
	}
}

BOOL appSetup()
{
	int i;

    for (i = 0; i < SWITCH_COUNT; i++) {
    	s_link_test[i].p_delay = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
		if (!s_link_test[i].p_delay) return FALSE;
		pifTimer_AttachEvtFinish(s_link_test[i].p_delay, _evtDelay, (void *)&c_link_request[i]);
    }

    if (!pifLink_Init(&s_link, PIF_ID_AUTO, &g_timer_1ms, LINK_T_SINGLE, 0, c_link_question)) return FALSE;
    pifLink_AttachUart(&s_link, &g_serial);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;		// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
