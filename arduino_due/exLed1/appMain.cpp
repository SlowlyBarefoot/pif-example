#include "appMain.h"
#include "exLed1.h"

#include "core/pif_log.h"
#include "display/pif_led.h"


PifTimerManager g_timer_1ms;
PifComm g_comm_log;

static PifLed s_led_l;
static PifLed s_led_rgb;


static uint16_t _taskLed(PifTask *pstTask)
{
	static uint16_t usStep = 0;
	static uint16_t usTimer = 0;

	(void)pstTask;

	if (!usTimer) {
		switch (usStep) {
		case 0:
			pifLed_PartOn(&s_led_rgb, 1 << 0);
			usStep = 1;
			usTimer = 6;
			break;

		case 1:
			pifLed_PartOff(&s_led_rgb, 1 << 0);
			pifLed_PartOn(&s_led_rgb, 1 << 1);
			usStep = 2;
			usTimer = 6;
			break;

		case 2:
			pifLed_PartOff(&s_led_rgb, 1 << 1);
			pifLed_PartOn(&s_led_rgb, 1 << 2);
			usStep = 3;
			usTimer = 6;
			break;

		case 3:
			pifLed_PartOff(&s_led_rgb, 1 << 2);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 0, 0);
			usStep = 4;
			usTimer = 6;
			break;

		case 4:
			pifLed_MBlinkOff(&s_led_rgb, 1 << 0, 0, OFF);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 1, 1);
			usStep = 5;
			usTimer = 6;
			break;

		case 5:
			pifLed_MBlinkOff(&s_led_rgb, 1 << 1, 1, OFF);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 2, 2);
			usStep = 6;
			usTimer = 6;
			break;

		case 6:
			pifLed_MBlinkOn(&s_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 2, 2);
			usStep = 7;
			usTimer = 6;
			break;

		case 7:
			pifLed_MBlinkOn(&s_led_rgb, 1 << 0, 0);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOff(&s_led_rgb, 1 << 2, 2, OFF);
			usStep = 8;
			usTimer = 6;
			break;

		case 8:
			pifLed_MBlinkOn(&s_led_rgb, 1 << 0, 0);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 1, 1);
			pifLed_MBlinkOn(&s_led_rgb, 1 << 2, 2);
			usStep = 9;
			usTimer = 6;
			break;

		case 9:
			pifLed_MBlinkOff(&s_led_rgb, 1 << 0, 0, OFF);
			pifLed_MBlinkOff(&s_led_rgb, 1 << 1, 1, OFF);
			pifLed_MBlinkOff(&s_led_rgb, 1 << 2, 2, OFF);
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

void appSetup()
{
    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;		// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;		// 1ms
	g_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;							// 500ms

    if (!pifLed_Init(&s_led_rgb, PIF_ID_AUTO, &g_timer_1ms, 3, actLedRGBState)) return;
    if (!pifLed_AttachMBlink(&s_led_rgb, 100, 3, 1, 2, 3)) return;				// 100ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 200, _taskLed, NULL, TRUE)) return;	// 200ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
