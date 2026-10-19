#include "app_port.h"

#include "sensor/pif_max31855.h"


PifLed g_led_l;
PifSpiPort g_spi_port;
PifTimerManager g_timer_1ms;


static void _evtMax31855Measure(PifMax31855* p_owner, double temperature, PifIssuerP p_issuer)
{
	(void)p_issuer;

	pifLog_Printf(LT_INFO, "Temp=%f Int=%f", temperature, p_owner->_internal);
}

BOOL appSetup()
{
	static PifMax31855 s_max31855;

    if (!pifMax31855_Init(&s_max31855, PIF_ID_AUTO, &g_spi_port, NULL)) goto fail;
    if (!pifMax31855_StartMeasurement(&s_max31855, 500, _evtMax31855Measure, NULL)) goto fail;	// 500ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) goto fail;											// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            ex_max31855           ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
