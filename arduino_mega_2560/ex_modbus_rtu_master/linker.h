#ifndef LINKER_H
#define LINKER_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "protocol/pif_modbus_rtu_master.h"


extern PifLed g_led_l;
extern PifModbusRtuMaster g_modbus_master;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_modbus;


BOOL appSetup();


#endif	// LINKER_H
