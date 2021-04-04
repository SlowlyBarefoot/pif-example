#include "appMain.h"
#include "exGpio1.h"

#include "pifGpio.h"
#include "pifLog.h"
#include "pifTask.h"


#define GPIO_COUNT         		3
#define TASK_COUNT              1


static PIF_stGpio *s_pstGpioL = NULL;
static PIF_stGpio *s_pstGpioRG = NULL;
static PIF_stGpio *s_pstGpioSwitch = NULL;


static void _taskGpioTest(PIF_stTask *pstTask)
{
	static SWITCH swLedL = OFF;
	static uint8_t ucLedRG = 0;

	(void)pstTask;

	pifGpio_WriteAll(s_pstGpioL, swLedL);
	swLedL ^= 1;

	pifGpio_WriteAll(s_pstGpioRG, ucLedRG);
	ucLedRG = (ucLedRG + 1) & 3;

	pifLog_Printf(LT_enInfo, "Sw:%u", pifGpio_ReadAll(s_pstGpioSwitch));
}

void appSetup()
{
    pif_Init(NULL);

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifGpio_Init(GPIO_COUNT)) return;
    s_pstGpioL = pifGpio_AddOut(PIF_ID_AUTO, 1, actGpioLedL);
    if (!s_pstGpioL) return;

    s_pstGpioRG = pifGpio_AddOut(PIF_ID_AUTO, 2, actGpioLedRG);
    if (!s_pstGpioRG) return;

    s_pstGpioSwitch = pifGpio_AddIn(PIF_ID_AUTO, 1, actGpioSwitch);
    if (!s_pstGpioSwitch) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriodMs(500, _taskGpioTest, NULL)) return;				// 500ms
}
