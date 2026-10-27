#include "app_port.h"


PifLed g_led_l;
PifLed g_led_rgb;
PifTimerManager g_timer1ms;


static uint32_t _taskLed(PifTask *p_task)
{
	static uint16_t step = 0;
	static uint16_t timer = 0;

	(void)p_task;

	if (!timer) {
		switch (step) {
		case 0:
			pifLed_PartOn(&g_led_rgb, 1 << 0);
			step = 1;
			timer = 6;
			break;

		case 1:
			pifLed_PartOff(&g_led_rgb, 1 << 0);
			pifLed_PartOn(&g_led_rgb, 1 << 1);
			step = 2;
			timer = 6;
			break;

		case 2:
			pifLed_PartOff(&g_led_rgb, 1 << 1);
			pifLed_PartOn(&g_led_rgb, 1 << 2);
			step = 3;
			timer = 6;
			break;

		case 3:
			pifLed_PartOff(&g_led_rgb, 1 << 2);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 0, 0);
			step = 4;
			timer = 6;
			break;

		case 4:
			pifLed_MBlinkOff(&g_led_rgb, 1 << 0, 0, OFF);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 1, 1);
			step = 5;
			timer = 6;
			break;

		case 5:
			pifLed_MBlinkOff(&g_led_rgb, 1 << 1, 1, OFF);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 2, 2);
			step = 6;
			timer = 6;
			break;

		case 6:
			pifLed_MBlinkOn(&g_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 2, 2);
			step = 7;
			timer = 6;
			break;

		case 7:
			pifLed_MBlinkOn(&g_led_rgb, 1 << 0, 0);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOff(&g_led_rgb, 1 << 2, 2, OFF);
			step = 8;
			timer = 6;
			break;

		case 8:
			pifLed_MBlinkOn(&g_led_rgb, 1 << 0, 0);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOn(&g_led_rgb, 1 << 2, 2);
			step = 9;
			timer = 6;
			break;

		case 9:
			pifLed_MBlinkOff(&g_led_rgb, 1 << 0, 0, OFF);
			pifLed_MBlinkOff(&g_led_rgb, 1 << 1, 1, OFF);
			pifLed_MBlinkOff(&g_led_rgb, 1 << 2, 2, OFF);
			step = 0;
			timer = 6;
			break;
		}
	}
	else {
		timer--;
	}
	return 0;
}

BOOL appSetup()
{
    if (!pifLed_AttachMBlink(&g_led_rgb, 100, 3, 1, 2, 3)) return FALSE;							// 100ms
    g_led_rgb._p_task->name = "LedRGB";

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 200000, _taskLed, NULL, TRUE)) return FALSE;	// 200ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;											// 500ms
    g_led_l._p_task->name = "LedL";
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***             ex_led1              ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer1ms), TIMER_1MS_SIZE);
    return TRUE;
}
