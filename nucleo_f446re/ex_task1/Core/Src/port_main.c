#include "main.h"
#include "app_port.h"


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static uint32_t _taskLedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = OFF;

	(void)pstTask;

    if (!nCount) {
    	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, sw);
		sw ^= 1;

		nCount = 9999;
    }
    else nCount--;
    return 0;
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 50, _taskLedToggle, NULL, TRUE)) return FALSE;	// 50us
	return TRUE;
}
