#include "app_main.h"
#include "ex_storage.h"

#include "pif_log.h"
#include "pif_storage.h"


PifTimerManager g_timer_1ms;

static PifStorage s_storage;

static int _CmdFormat(int argc, char *argv[]);
static int _CmdAlloc(int argc, char *argv[]);
static int _CmdFree(int argc, char *argv[]);
static int _CmdWrite(int argc, char *argv[]);
static int _CmdRead(int argc, char *argv[]);
#ifdef __PIF_DEBUG__
static int _CmdPrintInfo(int argc, char *argv[]);
static int _CmdDump(int argc, char *argv[]);
#endif

const PifLogCmdEntry c_psCmdTable[] = {
		{ "format", _CmdFormat, "\nStorage format" },
		{ "alloc", _CmdAlloc, "\nStorage alloc" },
		{ "free", _CmdFree, "\nStorage free" },
		{ "write", _CmdWrite, "\nStorage write" },
		{ "read", _CmdRead, "\nStorage read" },
#ifdef __PIF_DEBUG__
		{ "info", _CmdPrintInfo, "\nStorage print info" },
		{ "dump", _CmdDump, "\nStorage dump" },
#endif
		{ NULL, NULL, NULL }
};


static int _CmdFormat(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pifStorage_Format(&s_storage);
	pifLog_Printf(LT_INFO, "Storage format\n");
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdAlloc(int argc, char *argv[])
{
	uint16_t id, size;

	if (argc == 1) {
		pifLog_Printf(LT_NONE, "\nalloc [id] [size]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		id = atoi(argv[1]);
		size = atoi(argv[2]);
		if (!pifStorage_Alloc(&s_storage, id, size)) {
			pifLog_Printf(LT_NONE, "\nalloc: failed E=%d", pif_error);
		}
#ifdef __PIF_DEBUG__
		else {
			pifStorage_PrintInfo(&s_storage, TRUE);
		}
#endif
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdFree(int argc, char *argv[])
{
	uint16_t id;

	if (argc == 1) {
		pifLog_Printf(LT_NONE, "\nfree [id]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		if (!pifStorage_Free(&s_storage, id)) {
			pifLog_Printf(LT_NONE, "\nfree: failed E=%d", pif_error);
		}
#ifdef __PIF_DEBUG__
		else {
			pifStorage_PrintInfo(&s_storage, TRUE);
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
	PifStorageDataInfo* p_data_info;

	if (argc == 1) {
		pifLog_Printf(LT_NONE, "\nwrite [id]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		p_data_info = pifStorage_GetDataInfo(&s_storage, id);
		if (!p_data_info) {
			pifLog_Printf(LT_NONE, "\nwrite: not alloc ID=%d E=%d", id, pif_error);
		}
		else {
			if (argc > 2) {
				value = atoi(argv[2]);
			}
			else {
				value = rand() & 0xFF;
			}
			p_buffer = (uint8_t*)malloc(p_data_info->size);
			if (p_buffer) {
				memset(p_buffer, value, p_data_info->size);
				if (!pifStorage_Write(&s_storage, p_data_info, p_buffer)) {
					pifLog_Printf(LT_NONE, "\nwrite: failed E=%d", pif_error);
				}
				else {
					pifLog_Printf(LT_NONE, "\nwrite: value = %Xh", value);
				}
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
	PifStorageDataInfo* p_data_info;

	if (argc == 1) {
		pifLog_Printf(LT_NONE, "\nread [id]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		p_data_info = pifStorage_GetDataInfo(&s_storage, id);
		if (!p_data_info) {
			pifLog_Printf(LT_NONE, "\nread: not alloc ID=%d EC=%d", id, pif_error);
		}
		else {
			size = p_data_info->size;
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, 0, size);
				if (!pifStorage_Read(&s_storage, p_data_info, p_buffer)) {
					pifLog_Printf(LT_NONE, "\nread: failed E=%d", pif_error);
				}
				else {
					for (i = 0; i < size; i++) {
						if (!(i % 16)) {
							pifLog_Printf(LT_NONE, "\n%04X: ", p_data_info->first_sector * s_storage._p_info->sector_size + i);
						}
						pifLog_Printf(LT_NONE, "%02X ", p_buffer[i]);
					}
				}
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
	(void)argc;
	(void)argv;

	if (argc == 1) {
		pifStorage_PrintInfo(&s_storage, FALSE);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (argv[1][0] == 'h') {
			pifStorage_PrintInfo(&s_storage, TRUE);
			return PIF_LOG_CMD_NO_ERROR;
		}
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdDump(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pifStorage_Dump(&s_storage, 0, STORAGE_VOLUME);
	return PIF_LOG_CMD_NO_ERROR;
}

#endif

static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	actLedL(sw);
	sw ^= 1;
}

void appSetup()
{
	static PifComm s_comm_log;
	PifTimer *pstTimer1ms;

	pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

	pifLog_Init();

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	s_comm_log.act_send_data = actLogSendData;
	s_comm_log.act_receive_data = actLogReceiveData;

	if (!pifLog_AttachComm(&s_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;	// 1000us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);										// 500ms

	if (!pifStorage_Init(&s_storage, PIF_ID_AUTO, 128, STORAGE_SECTOR_SIZE, STORAGE_VOLUME,
			actStorageRead, actStorageWrite, NULL)) return;
	if (!s_storage._is_format) {
		pifLog_Printf(LT_INFO, "Storage Init : EC=%d\n", pif_error);
		if (!pifStorage_Format(&s_storage)) {
			pifLog_Printf(LT_INFO, "Storage format failed\n");
		}
		else {
			pifLog_Printf(LT_INFO, "Storage format\n");
		}
	}

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));

#ifdef __PIF_DEBUG__
	if (s_storage._is_format) {
		pifStorage_PrintInfo(&s_storage, TRUE);
	}
#endif
}
