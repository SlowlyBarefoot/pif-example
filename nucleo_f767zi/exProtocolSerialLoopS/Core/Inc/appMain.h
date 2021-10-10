#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifPulse.h"

#include "exSerial1.h"
#include "exSerial2.h"


//#define PRINT_PACKET_DATA

#define SWITCH_COUNT          	2

#define PIF_ID_SWITCH			0x100

#define UART_FRAME_SIZE			8


extern PifPulse *g_pstTimer1ms;
extern PIF_stComm *g_pstCommLog;


void appSetup();


#endif	// APP_MAIN_H
