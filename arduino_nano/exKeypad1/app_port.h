#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "input/pif_keypad.h"


#define TASK_SIZE	3

#define	ROWS		4		// 행(rows) 개수
#define	COLS		4		// 열(columns) 개수


extern PifKeypad g_keypad;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
