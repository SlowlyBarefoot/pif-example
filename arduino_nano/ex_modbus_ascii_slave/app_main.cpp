#include "linker.h"

#include <stdlib.h>


// The data model of this sever will support coils addresses 0 to 100 and registers addresses from 0 to 32
#define COILS_ADDR_MAX 256
#define REGS_ADDR_MAX 32


PifLed g_led_l;
PifModbusAsciiSlave g_modbus_slave;
PifTimerManager g_timer_1ms;
PifUart g_uart_modbus;

// A single nmbs_bitfield variable can keep 2000 coils
PifModbusBitField server_discrete_inputs[COILS_ADDR_MAX / 8] = {0};
PifModbusBitField server_coils[COILS_ADDR_MAX / 8] = {0};
uint16_t server_input_registers[REGS_ADDR_MAX] = {0};
uint16_t server_holding_registers[REGS_ADDR_MAX] = {0};


BOOL appSetup()
{
    int i;

    for (i = 0; i < COILS_ADDR_MAX / 8; i++) {
    	server_discrete_inputs[i] = COILS_ADDR_MAX / 8 - i;
    	server_coils[i] = i;
    }
    for (i = 0; i < REGS_ADDR_MAX; i++) {
    	server_holding_registers[i] = i;
    	server_input_registers[i] = REGS_ADDR_MAX - i;
    }

	pifModbusSlave_AttachDiscreteInput(&g_modbus_slave.parent, server_discrete_inputs, COILS_ADDR_MAX,
			NULL);
	pifModbusSlave_AttachCoils(&g_modbus_slave.parent, server_coils, COILS_ADDR_MAX,
			NULL, NULL, NULL);
	pifModbusSlave_AttachInputRegisters(&g_modbus_slave.parent, server_input_registers, REGS_ADDR_MAX,
			NULL);
	pifModbusSlave_AttachHoldingRegisters(&g_modbus_slave.parent, server_holding_registers, REGS_ADDR_MAX,
			NULL, NULL, NULL);

	pifModbusAsciiSlave_AttachUart(&g_modbus_slave, &g_uart_modbus);

	if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;			// 500ms
	pifLed_SBlinkOn(&g_led_l, 1 << 0);
	return TRUE;
}
