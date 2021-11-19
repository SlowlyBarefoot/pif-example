#include "appMain.h"
#include "main.h"

#include "pif_led.h"
#include "pif_log.h"


#define SINGLE_SHOT


PifComm g_comm_log;
PifPmlcdI2c g_pmlcd_i2c;
PifTimerManager g_timer_1ms;

static PifLed s_led_l;


uint16_t _taskPmlcdI2c(PifTask *pstTask)
{
	static int nStep = 0;
	static int nNumber = 0;

	(void)pstTask;

	pifLog_Printf(LT_INFO, "Task:%u(%u) %lu", __LINE__, nStep, (*pif_act_timer1us)());
	switch (nStep) {
	case 0:
		pifPmlcdI2c_LeftToRight(&g_pmlcd_i2c);
		pifPmlcdI2c_SetCursor(&g_pmlcd_i2c, 0, 0);
		pifPmlcdI2c_Print(&g_pmlcd_i2c, "Hello World.");
		pifPmlcdI2c_SetCursor(&g_pmlcd_i2c, 0, 1);
		pifPmlcdI2c_Printf(&g_pmlcd_i2c, "Go Home : %d", nNumber);
		break;

	case 1:
		pifPmlcdI2c_ScrollDisplayLeft(&g_pmlcd_i2c);
		break;

	case 2:
		pifPmlcdI2c_ScrollDisplayRight(&g_pmlcd_i2c);
		break;

	case 3:
		pifPmlcdI2c_Clear(&g_pmlcd_i2c);
		break;

	case 4:
		pifPmlcdI2c_RightToLeft(&g_pmlcd_i2c);
		pifPmlcdI2c_SetCursor(&g_pmlcd_i2c, 15, 0);
		pifPmlcdI2c_Print(&g_pmlcd_i2c, "Hello World.");
		pifPmlcdI2c_SetCursor(&g_pmlcd_i2c, 15, 1);
		pifPmlcdI2c_Printf(&g_pmlcd_i2c, "Go Home : %d", nNumber);
		break;

	case 5:
		pifPmlcdI2c_Clear(&g_pmlcd_i2c);
		break;
	}
	nStep++;
	if (nStep > 5) nStep = 0;
	nNumber++;
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;									// 500ms

    if (!pifPmlcdI2c_Init(&g_pmlcd_i2c, PIF_ID_AUTO, 0x27)) return;
    g_pmlcd_i2c._i2c.act_write = actPmlcdI2cWrite;
#if 0
    pifI2c_ScanAddress(&g_pmlcd_i2c._i2c);
#else
    pifPmlcdI2c_Begin(&g_pmlcd_i2c, 2, PIF_PMLCD_DS_5x8);
    pifPmlcdI2c_Backlight(&g_pmlcd_i2c);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 3000, _taskPmlcdI2c, NULL, TRUE)) return;	// 3000ms
#endif

    pifLed_BlinkOn(&s_led_l, 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
