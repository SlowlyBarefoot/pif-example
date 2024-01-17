#ifndef LINKER_H
#define LINKER_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "protocol/pif_modbus_ascii_master.h"


extern PifLed g_led_l;
extern PifModbusAsciiMaster g_modbus_master;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_modbus;


#ifdef __cplusplus
extern "C" {
#endif

BOOL appSetup();

#ifdef __cplusplus
}
#endif


#endif	// LINKER_H
