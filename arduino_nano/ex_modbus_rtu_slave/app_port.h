#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "protocol/pif_modbus_rtu_slave.h"


#define TASK_SIZE				7
#define TIMER_1MS_SIZE			2


extern PifLed g_led_l;
extern PifModbusRtuSlave g_modbus_slave;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_modbus;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
