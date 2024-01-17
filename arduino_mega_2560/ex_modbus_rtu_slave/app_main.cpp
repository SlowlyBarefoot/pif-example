#include "linker.h"

#include <stdlib.h>


// The data model of this sever will support coils addresses 0 to 100 and registers addresses from 0 to 32
#define COILS_ADDR_MAX 256
#define REGS_ADDR_MAX 32


PifLed g_led_l;
PifModbusRtuSlave g_modbus_slave;
PifTimerManager g_timer_1ms;
PifUart g_uart_modbus;

// A single nmbs_bitfield variable can keep 2000 coils
PifModbusBitField server_discrete_inputs[COILS_ADDR_MAX / 8] = {0};
PifModbusBitField server_coils[COILS_ADDR_MAX / 8] = {0};
uint16_t server_input_registers[REGS_ADDR_MAX] = {0};
uint16_t server_holding_registers[REGS_ADDR_MAX] = {0};


static void _evtReadCoils(uint16_t address, uint16_t quantity)
{
	pifLog_Printf(LT_INFO, "Coils: Addr=%d(#%d) Qty=%d\nData:", address, address + 1, quantity);
	// Read our coils values into coils_out
	for (int i = 0; i < quantity; i++) {
		bool value = PIF_MODBUS_READ_BIT_FIELD(server_coils, address + i);
		pifLog_Printf(LT_NONE, " %d", value);
	}
	pifLog_Print(LT_NONE, "\n");
}

static void _evtReadDiscreteInputs(uint16_t address, uint16_t quantity)
{
	pifLog_Printf(LT_INFO, "Discrete Inputs: Addr=%d(#%d) Qty=%d\nData:", address, address + 1, quantity);
	// Read our coils values into coils_out
	for (int i = 0; i < quantity; i++) {
		bool value = PIF_MODBUS_READ_BIT_FIELD(server_discrete_inputs, address + i);
		pifLog_Printf(LT_NONE, " %d", value);
	}
	pifLog_Print(LT_NONE, "\n");
}

static void _evtReadHoldingRegisters(uint16_t address, uint16_t quantity)
{
	int i;

	pifLog_Printf(LT_INFO, "Holding Registers: Addr=%d(#%d) Qty=%d\nData:", address, address + 1, quantity);
	for (i = 0; i < quantity; i++) {
		pifLog_Printf(LT_NONE, " %5u", server_holding_registers[address + i]);
	}
	pifLog_Print(LT_NONE, "\n");
}

static void _evtReadInputRegisters(uint16_t address, uint16_t quantity)
{
	int i;

	pifLog_Printf(LT_INFO, "Input Registers: Addr=%d(#%d) Qty=%d\nData:", address, address + 1, quantity);
	for (i = 0; i < quantity; i++) {
		pifLog_Printf(LT_NONE, " %5u", server_input_registers[address + i]);
	}
	pifLog_Print(LT_NONE, "\n");
}

static BOOL _evtWriteSingleCoil(uint16_t address)
{
	pifLog_Printf(LT_INFO, "Coils: Addr=%d(#%d)\nData: %d\n", address, address + 1, PIF_MODBUS_READ_BIT_FIELD(server_coils, address));
	return TRUE;
}

static BOOL _evtWriteSingleRegister(uint16_t address, uint16_t value)
{
	pifLog_Printf(LT_INFO, "Holding Registers: Addr=%d(#%d)\nData: %d\n", address, address + 1, server_holding_registers[address]);
	return TRUE;
}

static BOOL _evtWriteMultipleCoils(uint16_t address, uint16_t quantity)
{
	int i;

	pifLog_Printf(LT_INFO, "Coils: Addr=%d(#%d) Qty=%d\nData:", address, address + 1, quantity);
	for (i = 0; i < quantity; i++) {
		pifLog_Printf(LT_NONE, " %d", PIF_MODBUS_READ_BIT_FIELD(server_coils, address + i));
	}
	pifLog_Print(LT_NONE, "\n");
	return TRUE;
}

static BOOL _evtWriteMultipleRegisters(uint16_t address, uint16_t quantity)
{
	int i;

	pifLog_Printf(LT_INFO, "Holding Registers: Addr=%d(#%d) Qty=%d\nData:", address, address + 1, quantity);
	for (i = 0; i < quantity; i++) {
		pifLog_Printf(LT_NONE, " %5u", server_holding_registers[address + i]);
	}
	pifLog_Print(LT_NONE, "\n");
	return TRUE;
}

static void _evtLogControlChar(char ch)
{
	pifLog_Printf(LT_INFO, "Contorl Char = %x\n", ch);
}

static int _CmdDumpDiscreteInputs(int argc, char *argv[])
{
	int i;

	pifLog_Printf(LT_NONE, "Coils: size=%d", COILS_ADDR_MAX);
    for (i = 0; i < COILS_ADDR_MAX; i++) {
    	if (!(i % 10)) {
        	pifLog_Printf(LT_NONE, "\n%5u:", i + 1);
    	}
    	pifLog_Printf(LT_NONE, " %u", PIF_MODBUS_READ_BIT_FIELD(server_discrete_inputs, i));
    }
	pifLog_Printf(LT_NONE, "\n");
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdDumpCoils(int argc, char *argv[])
{
	int i;

	pifLog_Printf(LT_NONE, "Coils: size=%d", COILS_ADDR_MAX);
    for (i = 0; i < COILS_ADDR_MAX; i++) {
    	if (!(i % 10)) {
        	pifLog_Printf(LT_NONE, "\n%5u:", i + 1);
    	}
    	pifLog_Printf(LT_NONE, " %u", PIF_MODBUS_READ_BIT_FIELD(server_coils, i));
    }
	pifLog_Printf(LT_NONE, "\n");
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdDumpHoldingRegisters(int argc, char *argv[])
{
	int i;

	pifLog_Printf(LT_NONE, "Registers: size=%d", REGS_ADDR_MAX);
    for (i = 0; i < REGS_ADDR_MAX; i++) {
    	if (!(i % 10)) {
        	pifLog_Printf(LT_NONE, "\n%5u:", i + 1);
    	}
    	pifLog_Printf(LT_NONE, " %5u", server_holding_registers[i]);
    }
	pifLog_Printf(LT_NONE, "\n");
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdDumpInputRegisters(int argc, char *argv[])
{
	int i;

	pifLog_Printf(LT_NONE, "Registers: size=%d", REGS_ADDR_MAX);
    for (i = 0; i < REGS_ADDR_MAX; i++) {
    	if (!(i % 10)) {
        	pifLog_Printf(LT_NONE, "\n%5u:", i + 1);
    	}
    	pifLog_Printf(LT_NONE, " %5u", server_input_registers[i]);
    }
	pifLog_Printf(LT_NONE, "\n");
	return PIF_LOG_CMD_NO_ERROR;
}

const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "ddi", _CmdDumpDiscreteInputs, "Dump discrete inputs", NULL },
	{ "dc", _CmdDumpCoils, "Dump coils", NULL },
	{ "dhr", _CmdDumpHoldingRegisters, "Dump holding registers", NULL },
	{ "dir", _CmdDumpInputRegisters, "Dump input registers", NULL },

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
    int i, line;

    for (i = 0; i < COILS_ADDR_MAX / 8; i++) {
    	server_discrete_inputs[i] = COILS_ADDR_MAX / 8 - i;
    	server_coils[i] = i;
    }
    for (i = 0; i < REGS_ADDR_MAX; i++) {
    	server_holding_registers[i] = i;
    	server_input_registers[i] = REGS_ADDR_MAX - i;
    }

    if (!pifLog_UseCommand(c_cmd_table, "\nDebug> ")) { line = __LINE__; goto fail; }
    pifLog_AttachEvent(_evtLogControlChar);

	pifModbusSlave_AttachDiscreteInput(&g_modbus_slave.parent, server_discrete_inputs, COILS_ADDR_MAX,
			_evtReadDiscreteInputs);
	pifModbusSlave_AttachCoils(&g_modbus_slave.parent, server_coils, COILS_ADDR_MAX,
			_evtReadCoils, _evtWriteSingleCoil, _evtWriteMultipleCoils);
	pifModbusSlave_AttachInputRegisters(&g_modbus_slave.parent, server_input_registers, REGS_ADDR_MAX,
			_evtReadInputRegisters);
	pifModbusSlave_AttachHoldingRegisters(&g_modbus_slave.parent, server_holding_registers, REGS_ADDR_MAX,
			_evtReadHoldingRegisters, _evtWriteSingleRegister, _evtWriteMultipleRegisters);

	pifModbusRtuSlave_AttachUart(&g_modbus_slave, &g_uart_modbus);

	if (!pifLed_AttachSBlink(&g_led_l, 500)) { line = __LINE__; goto fail; }			// 500ms
	pifLed_SBlinkOn(&g_led_l, 1 << 0);
	return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Setup failed. %d\n", line);
	return FALSE;
}
