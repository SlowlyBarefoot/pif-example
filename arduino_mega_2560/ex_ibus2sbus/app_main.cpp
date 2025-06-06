#include "app_main.h"

#include "rc/pif_rc_ibus.h"
#include "rc/pif_rc_sbus.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_ibus;
PifUart g_uart_sbus;

static PifRcIbus s_ibus;
static PifRcSbus s_sbus;

static uint16_t s_channel[10];


static void _evtIbusReceive(PifRc* p_owner, uint16_t* p_channel, PifIssuerP p_issuer)
{
	int i;
	static int step = 0;

	(void)p_owner;
	(void)p_issuer;

	for (i = 0; i < 10; i++) {
		s_channel[i] = (p_channel[i] - 880) / 0.625f;
	}
	if (step) step--;
	else {
		for (i = 0; i < 10; i++) {
			pifLog_Printf(LT_NONE, "%4d:%4d ", p_channel[i], s_channel[i]);
		}
		pifLog_Printf(LT_NONE, "\n");
		step = 60;
	}
}

static uint32_t _taskSbus(PifTask *p_task)
{
	PifRcSbus *p_sbus = (PifRcSbus *)p_task->_p_client;

	pifRcSbus_SendFrame(p_sbus, s_channel, 10);   // 10 channels
	return 0;
}

BOOL appSetup()
{
    if (!pifRcIbus_Init(&s_ibus, PIF_ID_AUTO)) return FALSE;
    pifRc_AttachEvtReceive(&s_ibus.parent, _evtIbusReceive, NULL);
    pifRcIbus_AttachUart(&s_ibus, &g_uart_ibus);

    if (!pifRcSbus_Init(&s_sbus, PIF_ID_AUTO)) return FALSE;
    pifRcSbus_AttachUart(&s_sbus, &g_uart_sbus);

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 7500, _taskSbus, &s_sbus, TRUE)) return FALSE;	// 7.5ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;											// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}

