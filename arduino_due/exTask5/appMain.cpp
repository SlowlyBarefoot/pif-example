#include "appMain.h"
#include "exTask5.h"

#include "core/pif_log.h"


PifUart g_uart_log;

static PifTask* s_task;
static int s_state = 0;


static uint16_t _taskLedToggle(PifTask *pstTask)
{
	static int count = 0;
	static BOOL sw = LOW;

	(void)pstTask;

	if (count) {
		actLedL(sw);
		sw ^= 1;

		count--;
	}
	else {
		switch (s_state) {
		case 0:
			pifTask_ChangeMode(s_task, TM_PERIOD_MS, 1);
			s_state++;
			break;

		case 1:
			pifTask_ChangeMode(s_task, TM_RATIO, 50);
			s_state = 0;
			break;
		}
		count = 100;
#ifdef __PIF_DEBUG__
		pifTaskManager_PrintRatioTable();
#endif
	}

    return 0;
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

	if (!pifUart_Init(&g_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_log, TM_PERIOD_MS, 1, NULL)) return;		// 1ms
	g_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&g_uart_log)) return;

	s_task = pifTaskManager_Add(TM_RATIO, 50, _taskLedToggle, NULL, TRUE);		// 50%
	if (!s_task) return;

#ifdef __PIF_DEBUG__
	pifTaskManager_PrintRatioTable();
#endif

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
}
