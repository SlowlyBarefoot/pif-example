#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "input/pif_keypad.h"


#define	ROWS	4		// 행(rows) 개수
#define	COLS	4		// 열(columns) 개수


extern PifKeypad g_keypad;


BOOL appSetup();


#endif	// APP_MAIN_H
