#include "app_main.h"

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
	PifMax31855 max31855;

	if (!pifMax31855_Init(&max31855, PIF_ID_AUTO, &g_spi_port)) return FALSE;
    if (!pifMax31855_StartMeasurement(&max31855, 500, _evtMax31855Measure)) return FALSE;	// 500ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;									// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
