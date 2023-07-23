#include "appMain.h"


PifLed g_led_l;
PifLed g_led_rgb;
PifTimerManager g_timer_1ms;

static uint16_t _taskLed(PifTask *pstTask)
{
	static uint16_t usStep = 0;
	static uint16_t usTimer = 0;

	(void)pstTask;

	if (!usTimer) {
		switch (usStep) {
		case 0:
			pifLed_PartOn(&g_led_rgb, 1 << 0);
			usStep = 1;
			usTimer = 6;
			break;

		case 1:
			pifLed_PartOff(&g_led_rgb, 1 << 0);
			pifLed_PartOn(&g_led_rgb, 1 << 1);
			usStep = 2;
			usTimer = 6;
			break;

		case 2:
			pifLed_PartOff(&g_led_rgb, 1 << 1);
			pifLed_PartOn(&g_led_rgb, 1 << 2);
			usStep = 3;
			usTimer = 6;
			break;

		case 3:
			pifLed_PartOff(&g_led_rgb, 1 << 2);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 0, 0);
			usStep = 4;
			usTimer = 6;
			break;

		case 4:
			pifLed_MBlinkOff(&g_led_rgb, 1 << 0, 0, OFF);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 1, 1);
			usStep = 5;
			usTimer = 6;
			break;

		case 5:
			pifLed_MBlinkOff(&g_led_rgb, 1 << 1, 1, OFF);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 2, 2);
			usStep = 6;
			usTimer = 6;
			break;

		case 6:
			pifLed_MBlinkOn(&g_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 2, 2);
			usStep = 7;
			usTimer = 6;
			break;

		case 7:
			pifLed_MBlinkOn(&g_led_rgb, 1 << 0, 0);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOff(&g_led_rgb, 1 << 2, 2, OFF);
			usStep = 8;
			usTimer = 6;
			break;

		case 8:
			pifLed_MBlinkOn(&g_led_rgb, 1 << 0, 0);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 2, 2);
			usStep = 9;
			usTimer = 6;
			break;

		case 9:
			pifLed_MBlinkOff(&g_led_rgb, 1 << 0, 0, OFF);
			pifLed_MBlinkOff(&g_led_rgb, 1 << 1, 1, OFF);
			pifLed_MBlinkOff(&g_led_rgb, 1 << 2, 2, OFF);
			usStep = 0;
			usTimer = 6;
			break;
		}
	}
	else {
		usTimer--;
	}
	return 0;
}

BOOL appSetup()
{
    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;							// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    if (!pifLed_AttachMBlink(&g_led_rgb, 100, 3, 1, 2, 3)) return FALSE;			// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 200, _taskLed, NULL, TRUE)) return FALSE;	// 200ms
    return TRUE;
}
