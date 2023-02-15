#include "appMain.h"
#include "exProtocolSerialLoopM.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "protocol/pif_protocol.h"
#include "sensor/pif_sensor_switch.h"

#include "exSerial1.h"
#include "exSerial2.h"


PifTimerManager g_timer_1ms;
PifComm g_comm_log;


void appSetup()
{
	static PifLed s_led_l;

	pif_Init(NULL);

    if (!pifTaskManager_Init(6)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 7)) return;			// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE, "CommLog")) return;	// 1ms
#ifdef USE_SERIAL
    g_comm_log.act_send_data = actLogSendData;
#endif
#ifdef USE_USART
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;
#endif

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;								// 500ms

    if (!exSerial1_Setup()) return;
    if (!exSerial2_Setup()) return;

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}

