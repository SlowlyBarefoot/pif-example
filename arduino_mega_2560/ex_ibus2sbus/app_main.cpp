#include "app_main.h"

#include "rc/pif_rc_ibus.h"
#include "rc/pif_rc_sbus.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_ibus;
PifUart g_uart_sbus;

static PifRcIbus s_ibus;
static PifRcSbus s_sbus;


static void _evtIbusReceive(PifRc* p_owner, uint16_t* channel, PifIssuerP p_issuer)
{
	int i;

	(void)p_owner;
	(void)p_issuer;

	for (i = 0; i < 10; i++) {
		channel[i] = (channel[i] - 880) / 0.625f;
	}
	pifRcSbus_SendFrame(&s_sbus, channel, 10);   // 10 channels
}

BOOL appSetup()
{
    if (!pifRcIbus_Init(&s_ibus, PIF_ID_AUTO)) return FALSE;
    pifRc_AttachEvtReceive(&s_ibus.parent, _evtIbusReceive, NULL);
    pifRcIbus_AttachUart(&s_ibus, &g_uart_ibus);

    if (!pifRcSbus_Init(&s_sbus, PIF_ID_AUTO)) return FALSE;
    pifRcSbus_AttachUart(&s_sbus, &g_uart_sbus);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;		// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}

