#include "appMain.h"


PifGpio g_gpio_l;
PifGpio g_gpio_rg;
PifGpio g_gpio_switch;


static uint16_t _taskGpioTest(PifTask *pstTask)
{
	static SWITCH swLedL = OFF;
	static uint8_t ucLedRG = 0;

	(void)pstTask;

	pifGpio_WriteAll(&g_gpio_l, swLedL);
	swLedL ^= 1;

	pifGpio_WriteAll(&g_gpio_rg, ucLedRG);
	ucLedRG = (ucLedRG + 1) & 3;

	pifLog_Printf(LT_INFO, "Sw:%u", pifGpio_ReadAll(&g_gpio_switch));
	return 0;
}

BOOL appSetup()
{
    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskGpioTest, NULL, TRUE)) return FALSE;	// 500ms
    return TRUE;
}
