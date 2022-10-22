#include "app_main.h"
#include "ex_ibus2sbus.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "protocol/pif_rc_ibus.h"
#include "protocol/pif_rc_sbus.h"


PifTimerManager g_timer_1ms;

static PifRcIbus s_ibus;
static PifLed s_led_l;
static PifRcSbus s_sbus;


static void _evtIbusReceive(PifRc* p_owner, uint16_t* channel)
{
	int i;

	(void)p_owner;

	for (i = 0; i < 10; i++) {
		channel[i] = (channel[i] - 880) / 0.625f;
	}
	pifRcSbus_SendFrame(&s_sbus, channel, 10);   // 10 channels
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	static PifComm s_comm_ibus;
	static PifComm s_comm_sbus;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
    s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

	if (!pifComm_Init(&s_comm_ibus, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_ibus, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_ibus.act_receive_data = actSerial1ReceiveData;

    if (!pifRcIbus_Init(&s_ibus, PIF_ID_AUTO)) return;
    s_ibus.evt_receive = _evtIbusReceive;
    pifRcIbus_AttachComm(&s_ibus, &s_comm_ibus);

	if (!pifComm_Init(&s_comm_sbus, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_sbus, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_sbus.act_send_data = actSerial2SendData;

    if (!pifRcSbus_Init(&s_sbus, PIF_ID_AUTO)) return;
    pifRcSbus_AttachComm(&s_sbus, &s_comm_sbus);

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;								// 500ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}

