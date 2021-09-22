#include "appMain.h"
#include "exGpio1.h"

#include "pifGpio.h"
#include "pifLog.h"
#include "pifTask.h"


#define COMM_COUNT              1
#define GPIO_COUNT         		3
#define TASK_COUNT              2


static PIF_stGpio *s_pstGpioL = NULL;
static PIF_stGpio *s_pstGpioRG = NULL;
static PIF_stGpio *s_pstGpioSwitch = NULL;


static uint16_t _taskGpioTest(PIF_stTask *pstTask)
{
	static SWITCH swLedL = OFF;
	static uint8_t ucLedRG = 0;

	(void)pstTask;

	pifGpio_WriteAll(s_pstGpioL, swLedL);
	swLedL ^= 1;

	pifGpio_WriteAll(s_pstGpioRG, ucLedRG);
	ucLedRG = (ucLedRG + 1) & 3;

	pifLog_Printf(LT_enInfo, "Sw:%u", pifGpio_ReadAll(s_pstGpioSwitch));
	return 0;
}

void appSetup()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifGpio_Init(GPIO_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_Add(TM_enPeriodMs, 1, pifComm_Task, pstCommLog, TRUE)) return;		// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstGpioL = pifGpio_AddOut(PIF_ID_AUTO, 1, actGpioLedL);
    if (!s_pstGpioL) return;

    s_pstGpioRG = pifGpio_AddOut(PIF_ID_AUTO, 2, actGpioLedRG);
    if (!s_pstGpioRG) return;

    s_pstGpioSwitch = pifGpio_AddIn(PIF_ID_AUTO, 1, actGpioSwitch);
    if (!s_pstGpioSwitch) return;

    if (!pifTask_Add(TM_enPeriodMs, 500, _taskGpioTest, NULL, TRUE)) return;		// 500ms
}
