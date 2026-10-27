#include "app_port.h"

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
	PifTask* p_task;
	static PifXmodem s_xmodem;

    p_task = pifSensorSwitch_AttachTaskAcquire(&g_push_switch, PIF_ID_AUTO, TM_PERIOD, 10000, TRUE);	// 10ms
    if (!p_task) return FALSE;
    p_task->name = "PushSw";
    pifSensor_AttachEvtChange(&g_push_switch.parent, _evtPushSwitchChange, &s_xmodem);

    if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, XT_CRC)) return FALSE;
    s_xmodem._p_task->name = "Xmodem";
    pifXmodem_AttachTransport(&s_xmodem, &g_serial.parent);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;														// 500ms
    g_led_l._p_task->name = "LedL";
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
