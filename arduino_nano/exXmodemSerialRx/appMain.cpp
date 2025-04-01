#include "appMain.h"

#include "protocol/pif_xmodem.h"


PifLed g_led_l;
PifSensorSwitch g_push_switch;
PifUart g_serial;
PifTimerManager g_timer_1ms;


static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	if (state) {
	    pifXmodem_ReadyReceive((PifXmodem*)p_issuer);
	}
}

BOOL appSetup()
{
	static PifXmodem s_xmodem;

    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch, TM_PERIOD, 10000, TRUE)) return FALSE;	// 10ms
    pifSensor_AttachEvtChange(&g_push_switch.parent, _evtPushSwitchChange, &s_xmodem);

    if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, &g_timer_1ms, XT_CRC)) return FALSE;
    pifXmodem_AttachUart(&s_xmodem, &g_serial);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;											// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
