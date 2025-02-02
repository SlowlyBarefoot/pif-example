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
	static PifMax31855 s_max31855;

    if (!pifMax31855_Init(&s_max31855, PIF_ID_AUTO, &g_spi_port, NULL)) return FALSE;
    if (!pifMax31855_StartMeasurement(&s_max31855, 500, _evtMax31855Measure)) return FALSE;	// 500ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;									// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
