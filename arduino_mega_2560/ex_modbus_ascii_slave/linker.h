#ifndef LINKER_H
#define LINKER_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "protocol/pif_modbus_ascii_slave.h"


extern PifLed g_led_l;
extern PifModbusAsciiSlave g_modbus_slave;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_modbus;


BOOL appSetup();


#endif	// LINKER_H
