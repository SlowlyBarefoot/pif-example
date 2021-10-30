#include "app_main.h"
#include "main.h"

#include "pif_led.h"
#include "pif_log.h"


PifPulse g_timer1ms;
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
			pifLed_EachOn(&s_led_rgb, 0);
			step = 1;
			timer = 6;
			break;

		case 1:
			pifLed_EachOff(&s_led_rgb, 0);
			pifLed_EachOn(&s_led_rgb, 1);
			step = 2;
			timer = 6;
			break;

		case 2:
			pifLed_EachOff(&s_led_rgb, 1);
			pifLed_EachOn(&s_led_rgb, 2);
			step = 3;
			timer = 6;
			break;

		case 3:
			pifLed_EachOff(&s_led_rgb, 2);
			pifLed_BlinkOn(&s_led_rgb, 0);
			step = 4;
			timer = 6;
			break;

		case 4:
			pifLed_BlinkOff(&s_led_rgb, 0);
			pifLed_BlinkOn(&s_led_rgb, 1);
			step = 5;
			timer = 6;
			break;

		case 5:
			pifLed_BlinkOff(&s_led_rgb, 1);
			pifLed_BlinkOn(&s_led_rgb, 2);
			step = 6;
			timer = 6;
			break;

		case 6:
			pifLed_BlinkOn(&s_led_rgb, 1);
			pifLed_BlinkOn(&s_led_rgb, 2);
			step = 7;
			timer = 6;
			break;

		case 7:
			pifLed_BlinkOn(&s_led_rgb, 0);
			pifLed_BlinkOn(&s_led_rgb, 1);
			pifLed_BlinkOff(&s_led_rgb, 2);
			step = 8;
			timer = 6;
			break;

		case 8:
			pifLed_BlinkOn(&s_led_rgb, 0);
			pifLed_BlinkOn(&s_led_rgb, 1);
			pifLed_BlinkOn(&s_led_rgb, 2);
			step = 9;
			timer = 6;
			break;

		case 9:
			pifLed_BlinkOff(&s_led_rgb, 0);
			pifLed_BlinkOff(&s_led_rgb, 1);
			pifLed_BlinkOff(&s_led_rgb, 2);
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
    pifLog_Init();

    if (!pifPulse_Init(&g_timer1ms, PIF_ID_AUTO, 1000)) return;					// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;		// 1ms
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;								// 500ms
    pifLed_BlinkOn(&s_led_l, 0);

    if (!pifLed_Init(&s_led_rgb, PIF_ID_AUTO, &g_timer1ms, 3, actLedRGBState)) return;
    if (!pifLed_AttachBlink(&s_led_rgb, 100)) return;							// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLed, NULL, TRUE)) return;	// 100ms
}
