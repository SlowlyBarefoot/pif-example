#include "appMain.h"
#include "main.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "protocol/pif_protocol.h"
#include "sensor/pif_sensor_switch.h"


PifTimerManager g_timer_1ms;
PifUart g_uart_log;


void appSetup()
{
	static PifLed s_led_l;

	pif_Init(NULL);

    if (!pifTaskManager_Init(6)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 7)) return;			// 1000us

	if (!pifUart_Init(&g_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_log, TM_PERIOD_MS, 1, "UartLog")) return;		// 1ms
	if (!pifUart_AllocTxBuffer(&g_uart_log, 64)) return;
	g_uart_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachUart(&g_uart_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;								// 500ms

    if (!exSerial1_Setup()) return;
    if (!exSerial2_Setup()) return;

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}

