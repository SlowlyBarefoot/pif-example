#include "appMain.h"
#include "exPmlcdI2c.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_pmlcd_i2c.h"


#define SINGLE_SHOT


PifTimerManager g_timer_1ms;

static PifI2cPort s_i2c_port;
static PifLed s_led_l;
static PifPmlcdI2c s_pmlcd_i2c;


uint16_t _taskPmlcdI2c(PifTask *pstTask)
{
	static int nStep = 0;
	static int nNumber = 0;

	(void)pstTask;

	pifLog_Printf(LT_INFO, "Task:%u(%u)", __LINE__, nStep);
	switch (nStep) {
	case 0:
		pifPmlcdI2c_LeftToRight(&s_pmlcd_i2c);
		pifPmlcdI2c_SetCursor(&s_pmlcd_i2c, 0, 0);
		pifPmlcdI2c_Print(&s_pmlcd_i2c, "Hello World.");
		pifPmlcdI2c_SetCursor(&s_pmlcd_i2c, 0, 1);
		pifPmlcdI2c_Printf(&s_pmlcd_i2c, "Go Home : %d", nNumber);
		break;

	case 1:
		pifPmlcdI2c_ScrollDisplayLeft(&s_pmlcd_i2c);
		break;

	case 2:
		pifPmlcdI2c_ScrollDisplayRight(&s_pmlcd_i2c);
		break;

	case 3:
		pifPmlcdI2c_DisplayClear(&s_pmlcd_i2c);
		break;

	case 4:
		pifPmlcdI2c_RightToLeft(&s_pmlcd_i2c);
		pifPmlcdI2c_SetCursor(&s_pmlcd_i2c, 15, 0);
		pifPmlcdI2c_Print(&s_pmlcd_i2c, "Hello World.");
		pifPmlcdI2c_SetCursor(&s_pmlcd_i2c, 15, 1);
		pifPmlcdI2c_Printf(&s_pmlcd_i2c, "Go Home : %d", nNumber);
		break;

	case 5:
		pifPmlcdI2c_DisplayClear(&s_pmlcd_i2c);
		break;
	}
	nStep++;
	if (nStep > 5) nStep = 0;
	nNumber++;
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;									// 500ms

    if (!pifI2cPort_Init(&s_i2c_port, PIF_ID_AUTO, 1, 16)) return;
    s_i2c_port.act_write = actI2cWrite;

    if (!pifPmlcdI2c_Init(&s_pmlcd_i2c, PIF_ID_AUTO, &s_i2c_port, 0x27)) return;
#if 0
    pifI2cPort_ScanAddress(&s_i2c_port);
#else
    if (!pifPmlcdI2c_Begin(&s_pmlcd_i2c, 2, PIF_PMLCD_DS_5x8)) return;
    if (!pifPmlcdI2c_Backlight(&s_pmlcd_i2c)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskPmlcdI2c, NULL, TRUE)) return;	// 1000ms
#endif

    pifLed_BlinkOn(&s_led_l, 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
