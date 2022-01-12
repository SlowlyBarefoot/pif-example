#include "app_main.h"
#include "main.h"

#include "pif_led.h"
#include "pif_log.h"


PifTimerManager g_timer1ms;
PifComm g_comm_log;

static PifLed s_led_l;
static PifLed s_led_rgb;


static uint16_t _taskLed(PifTask *p_task)
{
	static uint16_t step = 0;
	static uint16_t timer = 0;

	(void)p_task;

	if (!timer) {
		switch (step) {
		case 0:
			pifLed_PartOn(&s_led_rgb, 1 << 0);
			step = 1;
			timer = 6;
			break;

		case 1:
			pifLed_PartOff(&s_led_rgb, 1 << 0);
			pifLed_PartOn(&s_led_rgb, 1 << 1);
			step = 2;
			timer = 6;
			break;

		case 2:
			pifLed_PartOff(&s_led_rgb, 1 << 1);
			pifLed_PartOn(&s_led_rgb, 1 << 2);
			step = 3;
			timer = 6;
			break;

		case 3:
			pifLed_PartOff(&s_led_rgb, 1 << 2);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 0, 0);
			step = 4;
			timer = 6;
			break;

		case 4:
			pifLed_MBlinkOff(&s_led_rgb, 1 << 0, 0, OFF);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 1, 1);
			step = 5;
			timer = 6;
			break;

		case 5:
			pifLed_MBlinkOff(&s_led_rgb, 1 << 1, 1, OFF);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 2, 2);
			step = 6;
			timer = 6;
			break;

		case 6:
			pifLed_MBlinkOn(&s_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 2, 2);
			step = 7;
			timer = 6;
			break;

		case 7:
			pifLed_MBlinkOn(&s_led_rgb, 1 << 0, 0);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOff(&s_led_rgb, 1 << 2, 2, OFF);
			step = 8;
			timer = 6;
			break;

		case 8:
			pifLed_MBlinkOn(&s_led_rgb, 1 << 0, 0);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 2, 2);
			step = 9;
			timer = 6;
			break;

		case 9:
			pifLed_MBlinkOff(&s_led_rgb, 1 << 0, 0, OFF);
			pifLed_MBlinkOff(&s_led_rgb, 1 << 1, 1, OFF);
			pifLed_MBlinkOff(&s_led_rgb, 1 << 2, 2, OFF);
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

void appSetup()
{
    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer1ms, PIF_ID_AUTO, 1000, 2)) return;		// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;		// 1ms
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;							// 500ms

    if (!pifLed_Init(&s_led_rgb, PIF_ID_AUTO, &g_timer1ms, 3, actLedRGBState)) return;
    if (!pifLed_AttachMBlink(&s_led_rgb, 100, 3, 1, 2, 3)) return;				// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 200, _taskLed, NULL, TRUE)) return;	// 200ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer1ms));
}
