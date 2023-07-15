#include "app_main.h"
#include "ex_max31855.h"

#include "core/pif_log.h"
#include "communication/pif_spi.h"
#include "display/pif_led.h"
#include "sensor/pif_max31855.h"


PifUart g_uart_log;
PifTimerManager g_timer_1ms;


static void _evtMax31855Measure(PifMax31855* p_owner, double temperature, PifIssuerP p_issuer)
{
	(void)p_issuer;

	pifLog_Printf(LT_INFO, "Temp=%f Int=%f", temperature, p_owner->_internal);
}

void appSetup()
{
	static PifLed s_led_l;
	static PifMax31855 s_max31855;
	static PifSpiPort spi_port;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;				// 1000us

	if (!pifUart_Init(&g_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_log, TM_PERIOD_MS, 1, NULL)) return;				// 1ms
	g_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&g_uart_log)) return;

    if (!pifSpiPort_Init(&spi_port, PIF_ID_AUTO, 1, 16)) return;
    spi_port.act_transfer = actTransfer;

    if (!pifMax31855_Init(&s_max31855, PIF_ID_AUTO, &spi_port, NULL)) return;
    if (!pifMax31855_StartMeasurement(&s_max31855, 500, _evtMax31855Measure)) return;	// 500ms

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;									// 500ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
