#include "appMain.h"
#include "exGpio1.h"

#include "pifGpio.h"
#include "pifLog.h"
#include "pifTask.h"


static PifGpio *s_pstGpioL = NULL;
static PifGpio *s_pstGpioRG = NULL;
static PifGpio *s_pstGpioSwitch = NULL;


static uint16_t _taskGpioTest(PifTask *pstTask)
{
	static SWITCH swLedL = OFF;
	static uint8_t ucLedRG = 0;

	(void)pstTask;

	pifGpio_WriteAll(s_pstGpioL, swLedL);
	swLedL ^= 1;

	pifGpio_WriteAll(s_pstGpioRG, ucLedRG);
	ucLedRG = (ucLedRG + 1) & 3;

	pifLog_Printf(LT_INFO, "Sw:%u", pifGpio_ReadAll(s_pstGpioSwitch));
	return 0;
}

void appSetup()
{
	PifComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstGpioL = pifGpio_Create(PIF_ID_AUTO, 1);
    if (!s_pstGpioL) return;
    pifGpio_AttachActOut(s_pstGpioL, actGpioLedL);

    s_pstGpioRG = pifGpio_Create(PIF_ID_AUTO, 2);
    if (!s_pstGpioRG) return;
    pifGpio_AttachActOut(s_pstGpioRG, actGpioLedRG);

    s_pstGpioSwitch = pifGpio_Create(PIF_ID_AUTO, 1);
    if (!s_pstGpioSwitch) return;
    pifGpio_AttachActIn(s_pstGpioSwitch, actGpioSwitch);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskGpioTest, NULL, TRUE)) return;	// 500ms
}
