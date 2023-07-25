#include "app_main.h"

#include "rc/pif_rc_ibus.h"
#include "rc/pif_rc_spektrum.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_ibus;
PifUart g_uart_spektrum;

static PifRcIbus s_ibus;
static PifRcSpektrum s_spektrum;


static void _evtIbusReceive(PifRc* p_parent, uint16_t* p_channel, PifIssuerP p_issuer)
{
	int i;
	uint16_t channel[PIF_SPEKTRUM_CHANNEL_COUNT];

	(void)p_parent;
	(void)p_issuer;

	for (i = 0; i < PIF_SPEKTRUM_CHANNEL_COUNT; i++) {
		channel[i] = (p_channel[i] - 988) * s_spektrum._pos_factor;
	}
	pifRcSpektrum_SendFrame(&s_spektrum, channel, PIF_SPEKTRUM_CHANNEL_COUNT);
}

BOOL appSetup()
{
    if (!pifRcIbus_Init(&s_ibus, PIF_ID_AUTO)) return FALSE;
    pifRc_AttachEvtReceive(&s_ibus.parent, _evtIbusReceive, NULL);
    pifRcIbus_AttachUart(&s_ibus, &g_uart_ibus);

//    if (!pifRcSpektrum_Init(&s_spektrum, PIF_ID_AUTO, PIF_SPEKTRUM_PROTOCOL_ID_22MS_2048_DSMS)) return FALSE;
	if (!pifRcSpektrum_Init(&s_spektrum, PIF_ID_AUTO, PIF_SPEKTRUM_PROTOCOL_ID_22MS_1024_DSM2)) return FALSE;
	pifRcSpektrum_AttachUart(&s_spektrum, &g_uart_spektrum);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;				// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}

