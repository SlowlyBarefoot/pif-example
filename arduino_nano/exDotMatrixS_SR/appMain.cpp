#include "appMain.h"
#include "exDotMatrixS_SR.h"

#include "core/pif_log.h"
#include "display/pif_dot_matrix.h"


PifTimerManager g_timer_1ms;

static PifDotMatrix s_dot_matrix;

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


static uint16_t _taskDotMatrixTest(PifTask *pstTask)
{
	static int nBlink = 0;
	static int index = 0;

	(void)pstTask;

	pifDotMatrix_SelectPattern(&s_dot_matrix, index);
	index++;
	if (index >= 10) index = 0;

	nBlink++;
	switch (nBlink) {
	case 10:
	    pifDotMatrix_BlinkOn(&s_dot_matrix, 200);
	    break;

	case 20:
	    pifDotMatrix_BlinkOff(&s_dot_matrix);
	    nBlink = 0;
	    break;
	}
	return 0;
}

void appSetup()
{
	static PifUart s_uart_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;					// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
    s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifDotMatrix_Init(&s_dot_matrix, PIF_ID_AUTO, &g_timer_1ms, 8, 8, actDotMatrixDisplay)) return;
    if (!pifDotMatrix_SetPatternSize(&s_dot_matrix, 10)) return;
    for (int i = 0; i < 10; i++) {
    	if (!pifDotMatrix_AddPattern(&s_dot_matrix, 8, 8, (uint8_t *)font8x8_basic[i])) return;
    }

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms
	if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskDotMatrixTest, NULL, TRUE)) return;	// 1000ms

	pifDotMatrix_Start(&s_dot_matrix);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
