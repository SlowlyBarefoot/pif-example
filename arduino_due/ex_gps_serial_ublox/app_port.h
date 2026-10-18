#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


#define TASK_SIZE				9
#define TIMER_1MS_SIZE			1

#define UART_GPS_SETUP_BAUDRATE	57600		// The baudrate the GPS is switched to by appSetup()


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_gps;

extern int g_print_data;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
