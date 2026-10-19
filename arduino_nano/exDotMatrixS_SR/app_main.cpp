#include "app_port.h"


PifDotMatrix g_dot_matrix;
PifTimerManager g_timer_1ms;

const uint8_t font8x8_basic[10][8] = {
    { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
    { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
    { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
    { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
    { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
    { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
    { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
    { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
    { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
    { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
};


static uint32_t _taskDotMatrixTest(PifTask* p_task)
{
	static int blink = 0;
	static int index = 0;

	(void)p_task;

	pifDotMatrix_SelectPattern(&g_dot_matrix, index);
	index++;
	if (index >= 10) index = 0;

	blink++;
	switch (blink) {
	case 10:
	    pifDotMatrix_BlinkOn(&g_dot_matrix, 200);
	    break;

	case 20:
	    pifDotMatrix_BlinkOff(&g_dot_matrix);
	    blink = 0;
	    break;
	}
	return 0;
}

BOOL appSetup()
{
    if (!pifDotMatrix_SetPatternSize(&g_dot_matrix, 10)) goto fail;
    for (int i = 0; i < 10; i++) {
    	if (!pifDotMatrix_AddPattern(&g_dot_matrix, 8, 8, (uint8_t *)font8x8_basic[i])) goto fail;
    }

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000000, _taskDotMatrixTest, NULL, TRUE)) goto fail;	// 1000ms

	pifDotMatrix_Start(&g_dot_matrix);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***          exDotMatrixS_SR         ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
