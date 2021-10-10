#include "appMain.h"
#include "exDotMatrixS_SR.h"

#include "pifDotMatrix.h"
#include "pifLog.h"


PifPulse *g_pstTimer1ms = NULL;

static PIF_stDotMatrix *s_pstDotMatrix = NULL;

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

	pifDotMatrix_SelectPattern(s_pstDotMatrix, index);
	index++;
	if (index >= 10) index = 0;

	nBlink++;
	switch (nBlink) {
	case 10:
	    pifDotMatrix_BlinkOn(s_pstDotMatrix, 200);
	    break;

	case 20:
	    pifDotMatrix_BlinkOff(s_pstDotMatrix);
	    nBlink = 0;
	    break;
	}
	return 0;
}

void appSetup()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);										// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;					// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;					// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstDotMatrix = pifDotMatrix_Create(PIF_ID_AUTO, g_pstTimer1ms, 8, 8, actDotMatrixDisplay);
    if (!s_pstDotMatrix) return;
	if (!pifDotMatrix_AttachTask(s_pstDotMatrix, TM_RATIO, 5, TRUE)) return;				// 5%
    if (!pifDotMatrix_SetPatternSize(s_pstDotMatrix, 10)) return;
    for (int i = 0; i < 10; i++) {
    	if (!pifDotMatrix_AddPattern(s_pstDotMatrix, 8, 8, (uint8_t *)font8x8_basic[i])) return;
    }

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms
	if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskDotMatrixTest, NULL, TRUE)) return;	// 1000ms

	pifDotMatrix_Start(s_pstDotMatrix);
}
