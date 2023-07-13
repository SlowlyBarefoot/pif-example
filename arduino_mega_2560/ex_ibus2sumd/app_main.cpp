#include "app_main.h"
#include "ex_ibus2sumd.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "rc/pif_rc_ibus.h"
#include "rc/pif_rc_sumd.h"


PifTimerManager g_timer_1ms;

static PifRcIbus s_ibus;
static PifLed s_led_l;
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

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	static PifComm s_comm_ibus;
	static PifComm s_comm_sumd;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;				// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, "CommLog")) return;			// 1ms
    s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

	if (!pifComm_Init(&s_comm_ibus, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_ibus, TM_PERIOD_MS, 1, "CommIbus")) return;			// 1ms
	s_comm_ibus.act_receive_data = actSerial1ReceiveData;

    if (!pifRcIbus_Init(&s_ibus, PIF_ID_AUTO)) return;
    pifRc_AttachEvtReceive(&s_ibus.parent, _evtIbusReceive, NULL);
    pifRcIbus_AttachComm(&s_ibus, &s_comm_ibus);

	if (!pifComm_Init(&s_comm_sumd, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_sumd, TM_PERIOD_MS, 1, "CommSumd")) return;			// 1ms
    s_comm_sumd.act_send_data = actSerial2SendData;

    if (!pifRcSumd_Init(&s_sumd, PIF_ID_AUTO)) return;
    pifRcSumd_AttachComm(&s_sumd, &s_comm_sumd);

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;									// 500ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}

