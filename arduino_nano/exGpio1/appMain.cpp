#include "appMain.h"
#include "exGpio1.h"

#include "core/pif_gpio.h"
#include "core/pif_log.h"
#include "core/pif_task.h"


static PifGpio s_gpio_l;
static PifGpio s_gpio_rg;
static PifGpio s_gpio_switch;


static uint16_t _taskGpioTest(PifTask *pstTask)
{
	static SWITCH swLedL = OFF;
	static uint8_t ucLedRG = 0;

	(void)pstTask;

	pifGpio_WriteAll(&s_gpio_l, swLedL);
	swLedL ^= 1;

	pifGpio_WriteAll(&s_gpio_rg, ucLedRG);
	ucLedRG = (ucLedRG + 1) & 3;

	pifLog_Printf(LT_INFO, "Sw:%u", pifGpio_ReadAll(&s_gpio_switch));
	return 0;
}

void appSetup()
{
	static PifComm s_comm_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(2)) return;

    pifLog_Init();

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, NULL)) return;	 	   // 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifGpio_Init(&s_gpio_l, PIF_ID_AUTO, 1)) return;
    pifGpio_AttachActOut(&s_gpio_l, actGpioLedL);

    if (!pifGpio_Init(&s_gpio_rg, PIF_ID_AUTO, 2)) return;
    pifGpio_AttachActOut(&s_gpio_rg, actGpioLedRG);

    if (!pifGpio_Init(&s_gpio_switch, PIF_ID_AUTO, 1)) return;
    pifGpio_AttachActIn(&s_gpio_switch, actGpioSwitch);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskGpioTest, NULL, TRUE)) return;	// 500ms

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
}
