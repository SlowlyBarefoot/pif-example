#include "linker.h"

#include <stdio.h>
#include <stdlib.h>


// The data model of this sever will support coils addresses 0 to 100 and registers addresses from 0 to 32
#define COILS_ADDR_MAX 32
#define REGS_ADDR_MAX 32


PifLed g_led_l;
PifModbusRtuMaster g_modbus_master;
PifTimerManager g_timer_1ms;
PifUart g_uart_modbus;

static PifModbusBitField coils[COILS_ADDR_MAX] = {0};


static void _evtLogControlChar(char ch)
{
	pifLog_Printf(LT_INFO, "Contorl Char = %x", ch);
}

static int _CmdReadCoils(int argc, char *argv[])
{
	int i, slave, addr, qty;

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "rc slave addr qty\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		slave = atoi(argv[0]);
		addr = atoi(argv[1]);
		qty = atoi(argv[2]);
		if (qty > 0 && addr + qty < COILS_ADDR_MAX * 8) {
			PIF_MODBUS_RESET_BIT_FIELD(coils);
			if (!pifModbusRtuMaster_ReadCoils(&g_modbus_master, slave, addr, qty, coils)) return PIF_LOG_CMD_INVALID_ARG;

			pifLog_Printf(LT_INFO, "Coils: Addr=%d Qty=%d UID=%d\nData:", addr, qty, slave);
			for (i = 0; i < qty; i++) {
				pifLog_Printf(LT_NONE, " %u", PIF_MODBUS_READ_BIT_FIELD(coils, i));
			}
			pifLog_Print(LT_NONE, "\n");
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdReadDiscreteInputs(int argc, char *argv[])
{
	int i, slave, addr, qty;

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "rdi slave ddr qty\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		slave = atoi(argv[0]);
		addr = atoi(argv[1]);
		qty = atoi(argv[2]);
		if (qty > 0 && addr + qty < COILS_ADDR_MAX * 8) {
			PIF_MODBUS_RESET_BIT_FIELD(coils);
			if (!pifModbusRtuMaster_ReadDiscreteInputs(&g_modbus_master, slave, addr, qty, coils)) return PIF_LOG_CMD_INVALID_ARG;

			pifLog_Printf(LT_INFO, "Discrete Inputs: Addr=%d Qty=%d UID=%d\nData:", addr, qty, slave);
			for (i = 0; i < qty; i++) {
				pifLog_Printf(LT_NONE, " %u", PIF_MODBUS_READ_BIT_FIELD(coils, i));
			}
			pifLog_Print(LT_NONE, "\n");
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdReadHoldingRegs(int argc, char *argv[])
{
	int i, slave, addr, qty;
	uint16_t r_regs[REGS_ADDR_MAX];

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "rhr slave addr qty\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		slave = atoi(argv[0]);
		addr = atoi(argv[1]);
		qty = atoi(argv[2]);
		if (qty > 0 && addr + qty < REGS_ADDR_MAX) {
			if (!pifModbusRtuMaster_ReadHoldingRegisters(&g_modbus_master, slave, addr, qty, r_regs)) return PIF_LOG_CMD_INVALID_ARG;

			pifLog_Printf(LT_INFO, "Regs: Addr=%d Qty=%d UID=%d\nData:", addr, qty, slave);
			for (i = 0; i < qty; i++) {
				pifLog_Printf(LT_NONE, " %5u", r_regs[i]);
			}
			pifLog_Print(LT_NONE, "\n");
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdReadInputRegs(int argc, char *argv[])
{
	int i, slave, addr, qty;
	uint16_t r_regs[REGS_ADDR_MAX];

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "rir slave addr qty\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		slave = atoi(argv[0]);
		addr = atoi(argv[1]);
		qty = atoi(argv[2]);
		if (qty > 0 && addr + qty < REGS_ADDR_MAX) {
			if (!pifModbusRtuMaster_ReadInputRegisters(&g_modbus_master, slave, addr, qty, r_regs)) return PIF_LOG_CMD_INVALID_ARG;

			pifLog_Printf(LT_INFO, "Regs: Addr=%d Qty=%d UID=%d\nData:", addr, qty, slave);
			for (i = 0; i < qty; i++) {
				pifLog_Printf(LT_NONE, " %5u", r_regs[i]);
			}
			pifLog_Print(LT_NONE, "\n");
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdWriteSingleCoil(int argc, char *argv[])
{
	int slave, addr;
	BOOL value;

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "wsc slave addr value\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		slave = atoi(argv[0]);
		addr = atoi(argv[1]);
		value = atoi(argv[2]);
		pifLog_Printf(LT_INFO, "Coils: Addr=%d UID=%d\nData: %u\n", addr, slave, value);

		if (!pifModbusRtuMaster_WriteSingleCoil(&g_modbus_master, slave, addr, value)) return PIF_LOG_CMD_INVALID_ARG;
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdWriteSingleReg(int argc, char *argv[])
{
	uint16_t slave, addr, value;

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "wsr slave addr value\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		slave = atoi(argv[0]);
		addr = atoi(argv[1]);
		value = atoi(argv[2]);
		pifLog_Printf(LT_INFO, "Regs: Addr=%d UID=%d\nData: %5u\n", addr, slave, value);

		if (!pifModbusRtuMaster_WriteSingleRegister(&g_modbus_master, slave, addr, value)) return PIF_LOG_CMD_INVALID_ARG;
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdWriteMultipleCoils(int argc, char *argv[])
{
	int i, slave, addr, qty;

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "wmc slave addr qty\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		slave = atoi(argv[0]);
		addr = atoi(argv[1]);
		qty = atoi(argv[2]);
		if (qty > 0 && addr + qty < COILS_ADDR_MAX * 8) {
			pifLog_Printf(LT_INFO, "Coils: Addr=%d Qty=%d UID=%d\nData:", addr, qty, slave);
			for (i = 0; i < qty; i++) {
				PIF_MODBUS_WRITE_BIT_FIELD(coils, i, rand() & 1);
				pifLog_Printf(LT_NONE, " %u", PIF_MODBUS_READ_BIT_FIELD(coils, i));
			}
			pifLog_Print(LT_NONE, "\n");

			if (!pifModbusRtuMaster_WriteMultipleCoils(&g_modbus_master, slave, addr, qty, coils)) return PIF_LOG_CMD_INVALID_ARG;
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdWriteMultipleRegs(int argc, char *argv[])
{
	int i, slave, addr, qty;
	uint16_t w_regs[REGS_ADDR_MAX];

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "wmr slave addr qty\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		slave = atoi(argv[0]);
		addr = atoi(argv[1]);
		qty = atoi(argv[2]);
		if (qty > 0 && addr + qty < REGS_ADDR_MAX) {
			pifLog_Printf(LT_INFO, "Regs: Addr=%d Qty=%d UID=%d\nData:", addr, qty, slave);
			for (i = 0; i < qty; i++) {
				w_regs[i] = rand() & 0xFFFF;
				pifLog_Printf(LT_NONE, " %5u", w_regs[i]);
			}
			pifLog_Print(LT_NONE, "\n");

			if (!pifModbusRtuMaster_WriteMultipleRegisters(&g_modbus_master, slave, addr, qty, w_regs)) return PIF_LOG_CMD_INVALID_ARG;
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdReadWriteMultipleRegs(int argc, char *argv[])
{
	int i, slave, r_addr, w_addr, qty;
	uint16_t r_regs[REGS_ADDR_MAX];
	uint16_t w_regs[REGS_ADDR_MAX];

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "rwmr slave r_addr w_addr qty\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 3) {
		slave = atoi(argv[0]);
		r_addr = atoi(argv[1]);
		w_addr = atoi(argv[2]);
		qty = atoi(argv[3]);
		if (qty > 0 && r_addr + qty < REGS_ADDR_MAX && w_addr + qty < REGS_ADDR_MAX) {
			pifLog_Printf(LT_INFO, "Write Regs: Addr=%d Qty=%d UID=%d\nData:", w_addr, qty, slave);
			for (i = 0; i < qty; i++) {
				w_regs[i] = rand() & 0xFFFF;
				pifLog_Printf(LT_NONE, " %5u", w_regs[i]);
			}
			pifLog_Print(LT_NONE, "\n");

			if (!pifModbusRtuMaster_ReadWriteMultipleRegisters(&g_modbus_master, slave, r_addr, qty, r_regs, w_addr, qty, w_regs)) return PIF_LOG_CMD_INVALID_ARG;

			pifLog_Printf(LT_INFO, "Read Regs: Addr=%d Qty=%d UID=%d\nData:", r_addr, qty, slave);
			for (i = 0; i < qty; i++) {
				pifLog_Printf(LT_NONE, " %5u", r_regs[i]);
			}
			pifLog_Print(LT_NONE, "\n");
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "rc", _CmdReadCoils, "Read coils", NULL },
	{ "rdi", _CmdReadDiscreteInputs, "Read discrete inputs", NULL },
	{ "rhr", _CmdReadHoldingRegs, "Read holding registers", NULL },
	{ "rir", _CmdReadInputRegs, "Read input registers", NULL },
	{ "wsc", _CmdWriteSingleCoil, "Write single coil", NULL },
	{ "wsr", _CmdWriteSingleReg, "Write single register", NULL },
	{ "wmc", _CmdWriteMultipleCoils, "Write multiple coils", NULL },
	{ "wmr", _CmdWriteMultipleRegs, "Write multiple registers", NULL },
	{ "rwmr", _CmdReadWriteMultipleRegs, "Read/Write multiple registers", NULL },

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
	int line;

	if (!pifLog_UseCommand(c_cmd_table, "\nDebug> ")) { line = __LINE__; goto fail; }
	pifLog_AttachEvent(_evtLogControlChar);

	pifModbusRtuMaster_AttachUart(&g_modbus_master, &g_uart_modbus);

	if (!pifLed_AttachSBlink(&g_led_l, 500)) { line = __LINE__; goto fail; }			// 500ms
	pifLed_SBlinkOn(&g_led_l, 1 << 0);
	return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Setup failed. %d\n", line);
	return FALSE;
}
