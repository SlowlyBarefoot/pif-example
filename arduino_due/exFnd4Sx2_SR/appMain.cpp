#include "appMain.h"
#include "exFnd4Sx2_SR.h"

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
		int32_t nValue = rand() % 1400000 - 200000;
		if (nValue <= -100000) {
			pifFnd_SetString(&s_fnd, (char *)"AAAAAAAA");
		}
		else if (nValue < 1000000) {
			pifFnd_SetInterger(&s_fnd, nValue);
		}
		else {
			pifFnd_SetString(&s_fnd, (char *)"BBBBBBBB");
		}

		pifLog_Printf(LT_INFO, "Blink:%d Float:%d Value:%d", swBlink, swFloat, nValue);
	}
	else {
		s_fnd.sub_numeric_digits = 2;
		double dValue = (rand() % 1100000 - 100000) / 100.0;
		pifFnd_SetFloat(&s_fnd, dValue);

		pifLog_Printf(LT_INFO, "Blink:%d Float:%d Value:%2f", swBlink, swFloat, dValue);
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
	static PifComm s_comm_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE, NULL)) return;		// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    pifFnd_SetUserChar(c_ucUserChar, 2);
    if (!pifFnd_Init(&s_fnd, PIF_ID_AUTO, &g_timer_1ms, 8, actFndDisplay)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 3000, _taskFndTest, NULL, TRUE)) return;	// 3000ms

    pifFnd_Start(&s_fnd);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
