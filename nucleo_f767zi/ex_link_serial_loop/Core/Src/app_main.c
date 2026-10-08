#include "app_main.h"

#include "ex_serial1.h"
#include "ex_serial2.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;


BOOL appSetup()
{
    if (!exSerial1_Setup()) return FALSE;
    if (!exSerial2_Setup()) return FALSE;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;		// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}

