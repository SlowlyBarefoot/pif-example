#include "appMain.h"
#include "exFnd4S_SR.h"

#include "core/pif_log.h"
#include "display/pif_fnd.h"


PifTimerManager g_timer_1ms;

static PifFnd s_fnd;

const uint8_t c_ucUserChar[] = { 0x01, 0x08 };


static uint16_t _taskFndTest(PifTask *pstTask)
{
	static int nBlink = 0;
	static BOOL swBlink = FALSE;
	static BOOL swFloat = FALSE;

	(void)pstTask;

	if (swFloat) {
		s_fnd.sub_numeric_digits = 0;
		int32_t nValue = rand() % 14000 - 2000;
		if (nValue <= -1000) {
			pifFnd_SetString(&s_fnd, (char *)"AAAA");
		}
		else if (nValue < 10000) {
			pifFnd_SetInterger(&s_fnd, nValue);
		}
		else {
			pifFnd_SetString(&s_fnd, (char *)"BBBB");
		}

		pifLog_Printf(LT_INFO, "Blink:%d Float:%d Value:%d", swBlink, swFloat, nValue);
	}
	else {
		s_fnd.sub_numeric_digits = 1;
		double dValue = (rand() % 11000 - 1000) / 10.0;
		pifFnd_SetFloat(&s_fnd, dValue);

		pifLog_Printf(LT_INFO, "Blink:%d Float:%d Value:%1f", swBlink, swFloat, dValue);
	}
	swFloat ^= 1;
	nBlink = (nBlink + 1) % 20;
	if (!nBlink) {
		if (swBlink) {
		    pifFnd_BlinkOff(&s_fnd);
		}
		else {
		    pifFnd_BlinkOn(&s_fnd, 200);
		}
		swBlink ^= 1;
	}
	return 0;
}

void appSetup()
{
	static PifUart s_uart_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
    s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return;

    pifFnd_SetUserChar(c_ucUserChar, 2);
    if (!pifFnd_Init(&s_fnd, PIF_ID_AUTO, &g_timer_1ms, 4, actFndDisplay)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskFndTest, NULL, TRUE)) return;	// 1000ms

    pifFnd_Start(&s_fnd);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
