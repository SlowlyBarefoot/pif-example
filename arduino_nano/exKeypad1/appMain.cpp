#include "appMain.h"
#include "exKeypad1.h"

#include "pif_keypad.h"
#include "pif_log.h"


const char c_cKeys[ROWS * COLS] = {
	'1', '4', '7', '*',
	'2', '5', '8', '0',
	'3', '6', '9', '#',
	'A', 'B', 'C', 'D'
};


static void _evtKeypadPressed(char cChar)
{
	pifLog_Printf(LT_INFO, "Keypad:Pressed %c", cChar);
}

static void _evtKeypadReleased(char cChar, uint32_t unOnTime)
{
	pifLog_Printf(LT_INFO, "Keypad:Released %c T:%lu", cChar, unOnTime);
}

static void _evtKeypadLongReleased(char cChar, uint32_t unOnTime)
{
	pifLog_Printf(LT_INFO, "Keypad:LongReleased %c T:%lu", cChar, unOnTime);
}

static void _evtKeypadDoublePressed(char cChar)
{
	pifLog_Printf(LT_INFO, "Keypad:DoublePressed %c", cChar);
}

void appSetup()
{
	PifComm *pstCommLog;
	PifKeypad *pstKeypad;

	pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

	pifLog_Init();

	pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    pstKeypad = pifKeypad_Create(PIF_ID_AUTO, ROWS * COLS, c_cKeys);
    if (!pstKeypad) return;
    pifKeypad_AttachAction(pstKeypad, actKeypadAcquire);
    pstKeypad->evt_pressed = _evtKeypadPressed;
    pstKeypad->evt_released = _evtKeypadReleased;
    pstKeypad->evt_long_released = _evtKeypadLongReleased;
    pstKeypad->evt_double_pressed = _evtKeypadDoublePressed;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
}
