#include "appMain.h"


PifKeypad g_keypad;

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

BOOL appSetup()
{
    if (!pifKeypad_SetKeymap(&g_keypad, ROWS * COLS, c_cKeys)) return FALSE;
	g_keypad.evt_pressed = _evtKeypadPressed;
    g_keypad.evt_released = _evtKeypadReleased;
    g_keypad.evt_long_released = _evtKeypadLongReleased;
    g_keypad.evt_double_pressed = _evtKeypadDoublePressed;
    if (!pifKeypad_Start(&g_keypad, NULL)) return FALSE;
    return TRUE;
}
