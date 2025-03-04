#include "appMain.h"


#define SINGLE_SHOT


PifI2cPort g_i2c_port;
PifLed g_led_l;
PifTimerManager g_timer_1ms;

static PifPmlcdI2c s_pmlcd_i2c;


static uint32_t _taskPmlcdI2c(PifTask *pstTask)
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

BOOL appSetup()
{
    if (!pifPmlcdI2c_Init(&s_pmlcd_i2c, PIF_ID_AUTO, &g_i2c_port, 0x27, NULL)) return FALSE;
    s_pmlcd_i2c._p_i2c->max_transfer_size = 32;
#if 0
    pifI2cPort_ScanAddress(&g_i2c_port);
#else
    if (!pifPmlcdI2c_Begin(&s_pmlcd_i2c, 2, PIF_PMLCD_DS_5x8)) return FALSE;
    if (!pifPmlcdI2c_Backlight(&s_pmlcd_i2c)) return FALSE;

    if (!pifTaskManager_Add(TM_PERIOD, 1000000, _taskPmlcdI2c, NULL, TRUE)) return FALSE;	// 1000ms
#endif

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;									// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
