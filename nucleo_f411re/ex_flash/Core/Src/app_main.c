#include "app_main.h"


PifFlash g_flash[FLASH_RECORD_COUNT];
PifTimerManager g_timer_1ms;

PifTimer* g_timer_led;


static int _CmdWrite(int argc, char *argv[]);
static int _CmdRead(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
		{ "help", pifLog_CmdHelp, "This command", NULL },
		{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
		{ "task", pifLog_CmdPrintTask, "Print task", NULL },
		{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
		{ "write", _CmdWrite, "Flash write", NULL },
		{ "read", _CmdRead, "Flash read", NULL },

		{ NULL, NULL, NULL, NULL }
};


static int _CmdWrite(int argc, char *argv[])
{
	uint8_t value;
	uint8_t* p_buffer;
	uint16_t id, size;

	if (argc == 0) {
		pifLog_Print(LT_INFO, "write [id] [size] [value]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[0]);
		size = atoi(argv[1]);
		if (id >= FLASH_RECORD_COUNT) return PIF_LOG_CMD_INVALID_ARG;
		if (argc > 2) {
			value = atoi(argv[2]);
		}
		else {
			value = rand() & 0xFF;
		}
		p_buffer = (uint8_t*)malloc(size);
		if (p_buffer) {
			memset(p_buffer, value, size);
			// The record is written from the start of its sector, which is erased when the record reaches it.
			// Write may be called any number of times in between, in pieces of any size.
			pifFlash_Begin(&g_flash[id]);
			pifFlash_Write(&g_flash[id], p_buffer, size);
			if (!pifFlash_End(&g_flash[id])) {
				pifLog_Printf(LT_INFO, "write: failed ID=%d E=%d", id, pif_error);
			}
			else {
				pifLog_Printf(LT_INFO, "write: value = %X", value);
			}
			free(p_buffer);
		}
		else return PIF_LOG_CMD_INVALID_ARG;
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdRead(int argc, char *argv[])
{
	uint8_t* p_buffer;
	uint16_t i, id, size;

	if (argc == 0) {
		pifLog_Print(LT_INFO, "read [id] [size]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[0]);
		size = atoi(argv[1]);
		if (id >= FLASH_RECORD_COUNT) return PIF_LOG_CMD_INVALID_ARG;
		p_buffer = (uint8_t*)malloc(size);
		if (p_buffer) {
			memset(p_buffer, 0, size);
			if (!pifFlash_Read(&g_flash[id], 0, p_buffer, size)) {
				pifLog_Printf(LT_INFO, "read: failed ID=%d E=%d", id, pif_error);
			}
			else {
				for (i = 0; i < size; i++) {
					if (!(i % 16)) {
						pifLog_Printf(LT_NONE, "\n%04X: ", i);
					}
					pifLog_Printf(LT_NONE, "%02X ", p_buffer[i]);
				}
				pifLog_Print(LT_NONE, "\n");
			}
			free(p_buffer);
		}
		else return PIF_LOG_CMD_INVALID_ARG;
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

BOOL appSetup()
{
	if (!pifLog_UseCommand(32, c_psCmdTable, "\nDebug> ")) return FALSE;	// 32bytes

	pifTimer_Start(g_timer_led, 500);										// 500ms
	return TRUE;
}
