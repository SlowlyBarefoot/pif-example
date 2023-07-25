#include "app_main.h"


PifI2cPort g_i2c_port;
PifStorageVar g_storage;
PifTimerManager g_timer_1ms;
PifTimer* g_timer_led;

static int _CmdFormat(int argc, char *argv[]);
static int _CmdCreate(int argc, char *argv[]);
static int _CmdDelete(int argc, char *argv[]);
static int _CmdWrite(int argc, char *argv[]);
static int _CmdRead(int argc, char *argv[]);
#ifdef __PIF_DEBUG__
static int _CmdPrintInfo(int argc, char *argv[]);
static int _CmdDump(int argc, char *argv[]);
#endif

const PifLogCmdEntry c_psCmdTable[] = {
		{ "help", pifLog_CmdHelp, "This command", NULL },
		{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
		{ "task", pifLog_CmdPrintTask, "Print task", NULL },
		{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
		{ "format", _CmdFormat, "Storage format", NULL },
		{ "create", _CmdCreate, "Storage create", NULL },
		{ "delete", _CmdDelete, "Storage delete", NULL },
		{ "write", _CmdWrite, "Storage write", NULL },
		{ "read", _CmdRead, "Storage read", NULL },
#ifdef __PIF_DEBUG__
		{ "info", _CmdPrintInfo, "Storage print info", NULL },
		{ "dump", _CmdDump, "Storage dump", NULL },
#endif
		{ NULL, NULL, NULL, NULL }
};


static int _CmdFormat(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	if (pifStorage_Format(&g_storage.parent)) {
		pifLog_Print(LT_INFO, "Storage format: Success\n");
	}
	else {
		pifLog_Printf(LT_INFO, "Storage format: Failed(%d)\n", pif_error);
	}
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdCreate(int argc, char *argv[])
{
	uint16_t id, size;

	if (argc == 0) {
		pifLog_Print(LT_INFO, "create [id] [size]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[0]);
		size = atoi(argv[1]);
		if (!pifStorage_Create(&g_storage.parent, id, size)) {
			pifLog_Printf(LT_INFO, "create: failed E=%d", pif_error);
		}
#ifdef __PIF_DEBUG__
		else {
			pifStorageVar_PrintInfo(&g_storage, TRUE);
		}
#endif
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdDelete(int argc, char *argv[])
{
	uint16_t id;

	if (argc == 0) {
		pifLog_Print(LT_INFO, "delete [id]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		id = atoi(argv[0]);
		if (!pifStorage_Delete(&g_storage.parent, id)) {
			pifLog_Printf(LT_INFO, "delete: failed E=%d", pif_error);
		}
#ifdef __PIF_DEBUG__
		else {
			pifStorageVar_PrintInfo(&g_storage, TRUE);
		}
#endif
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdWrite(int argc, char *argv[])
{
	uint8_t value;
	uint8_t* p_buffer;
	uint16_t id;
	PifStorageVarDataInfo* p_data_info;

	if (argc == 0) {
		pifLog_Print(LT_INFO, "write [id] [value]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		id = atoi(argv[0]);
		p_data_info = (PifStorageVarDataInfo*)pifStorage_Open(&g_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_INFO, "write: not alloc ID=%d E=%d", id, pif_error);
		}
		else {
			if (argc > 1) {
				value = atoi(argv[1]);
			}
			else {
				value = rand() & 0xFF;
			}
			p_buffer = (uint8_t*)malloc(p_data_info->size);
			if (p_buffer) {
				memset(p_buffer, value, p_data_info->size);
				if (!pifStorage_Write(&g_storage.parent, (PifStorageDataInfoP)p_data_info, p_buffer, p_data_info->size)) {
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
	PifStorageVarDataInfo* p_data_info;

	if (argc == 0) {
		pifLog_Print(LT_INFO, "read [id]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		id = atoi(argv[0]);
		p_data_info = (PifStorageVarDataInfo*)pifStorage_Open(&g_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_INFO, "read: not alloc ID=%d EC=%d", id, pif_error);
		}
		else {
			size = p_data_info->size;
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, 0, size);
				if (!pifStorage_Read(&g_storage.parent, p_buffer, (PifStorageDataInfoP)p_data_info, p_data_info->size)) {
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

#ifdef __PIF_DEBUG__

static int _CmdPrintInfo(int argc, char *argv[])
{
	if (argc == 0) {
		pifStorageVar_PrintInfo(&g_storage, FALSE);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		if (argv[0][0] == 'h') {
			pifStorageVar_PrintInfo(&g_storage, TRUE);
			return PIF_LOG_CMD_NO_ERROR;
		}
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdDump(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pifStorageVar_Dump(&g_storage, 0, EEPROM_VOLUME);
	return PIF_LOG_CMD_NO_ERROR;
}

#endif

BOOL appSetup()
{
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return FALSE;

    pifTimer_Start(g_timer_led, 500);									// 500ms

	if (!pifStorageVar_IsFormat(&g_storage.parent)) {
		pifLog_Printf(LT_INFO, "Storage Init : EC=%d", pif_error);
		if (!pifStorage_Format(&g_storage.parent)) {
			pifLog_Printf(LT_INFO, "Storage format failed");
		}
		else {
			pifLog_Printf(LT_INFO, "Storage format");
		}
	}

#ifdef __PIF_DEBUG__
	if (!pifStorageVar_IsFormat(&g_storage.parent)) {
		pifStorageVar_PrintInfo(&g_storage, TRUE);
	}
#endif
	return TRUE;
}
