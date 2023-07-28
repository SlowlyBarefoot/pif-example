#include "app_main.h"


PifStorageFix g_storage;
PifTimerManager g_timer_1ms;

PifTimer* g_timer_led;


static int _CmdWrite(int argc, char *argv[]);
static int _CmdRead(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
		{ "help", pifLog_CmdHelp, "This command", NULL },
		{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
		{ "task", pifLog_CmdPrintTask, "Print task", NULL },
		{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
		{ "write", _CmdWrite, "Storage write", NULL },
		{ "read", _CmdRead, "Storage read", NULL },

		{ NULL, NULL, NULL, NULL }
};


static int _CmdWrite(int argc, char *argv[])
{
	uint8_t value;
	uint8_t* p_buffer;
	uint16_t id, size;
	PifStorageFixDataInfo* p_data_info;

	if (argc == 0) {
		pifLog_Print(LT_INFO, "write [id] [size] [value]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[0]);
		size = atoi(argv[1]);
		p_data_info = (PifStorageFixDataInfo*)pifStorage_Open(&g_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_INFO, "write: not alloc ID=%d E=%d", id, pif_error);
		}
		else {
			if (argc > 2) {
				value = atoi(argv[2]);
			}
			else {
				value = rand() & 0xFF;
			}
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, value, size);
				if (!pifStorage_Write(&g_storage.parent, (PifStorageDataInfoP)p_data_info, p_buffer, size)) {
					pifLog_Printf(LT_INFO, "write: failed E=%d", pif_error);
				}
				else {
					pifLog_Printf(LT_INFO, "write: value = %X", value);
				}
				free(p_buffer);
			}
			else return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdRead(int argc, char *argv[])
{
	uint8_t* p_buffer;
	uint16_t i, id, size;
	PifStorageFixDataInfo* p_data_info;

	if (argc == 0) {
		pifLog_Print(LT_INFO, "read [id] [size]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[0]);
		size = atoi(argv[1]);
		p_data_info = (PifStorageFixDataInfo*)pifStorage_Open(&g_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_INFO, "read: not alloc ID=%d EC=%d", id, pif_error);
		}
		else {
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, 0, size);
				if (!pifStorage_Read(&g_storage.parent, p_buffer, (PifStorageDataInfoP)p_data_info, size)) {
					pifLog_Printf(LT_INFO, "read: failed E=%d", pif_error);
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
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

BOOL appSetup()
{
	if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return FALSE;

	pifTimer_Start(g_timer_led, 500);		// 500ms
	return TRUE;
}
