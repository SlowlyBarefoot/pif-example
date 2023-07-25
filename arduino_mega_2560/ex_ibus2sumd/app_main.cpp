#include "app_main.h"

#include "rc/pif_rc_ibus.h"
#include "rc/pif_rc_sumd.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_ibus;
PifUart g_uart_sumd;

static PifRcIbus s_ibus;
static PifRcSumd s_sumd;


static void _evtIbusReceive(PifRc* p_owner, uint16_t* channel, PifIssuerP p_issuer)
{
	int i;

	(void)p_owner;
	(void)p_issuer;

	for (i = 0; i < 10; i++) {
		channel[i] = channel[i] * 8;
	}
	pifRcSumd_SendFrame(&s_sumd, channel, 10);   // 10 channels
}

BOOL appSetup()
{
    if (!pifRcIbus_Init(&s_ibus, PIF_ID_AUTO)) return FALSE;
    pifRc_AttachEvtReceive(&s_ibus.parent, _evtIbusReceive, NULL);
    pifRcIbus_AttachUart(&s_ibus, &g_uart_ibus);

    if (!pifRcSumd_Init(&s_sumd, PIF_ID_AUTO)) return FALSE;
    pifRcSumd_AttachUart(&s_sumd, &g_uart_sumd);

	if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;			// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}

