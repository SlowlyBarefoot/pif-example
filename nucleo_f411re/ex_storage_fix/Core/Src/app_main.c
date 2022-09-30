#include "app_main.h"
#include "main.h"

#include "pif_storage_fix.h"


PifComm g_comm_log;
PifTimerManager g_timer_1ms;

static PifStorageFix s_storage;

static int _CmdWrite(int argc, char *argv[]);
static int _CmdRead(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
		{ "write", _CmdWrite, "Storage write" },
		{ "read", _CmdRead, "Storage read" },
		{ NULL, NULL, NULL }
};


static int _CmdWrite(int argc, char *argv[])
{
	uint8_t value;
	uint8_t* p_buffer;
	uint16_t id, size;
	PifStorageFixDataInfo* p_data_info;

	if (argc == 1) {
		pifLog_Print(LT_INFO, "write [id] [size] [value]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		id = atoi(argv[1]);
		size = atoi(argv[2]);
		p_data_info = (PifStorageFixDataInfo*)pifStorage_Open(&s_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_INFO, "write: not alloc ID=%d E=%d", id, pif_error);
		}
		else {
			if (argc > 3) {
				value = atoi(argv[3]);
			}
			else {
				value = rand() & 0xFF;
			}
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, value, size);
				if (!pifStorage_Write(&s_storage.parent, (PifStorageDataInfoP)p_data_info, p_buffer, size)) {
					pifLog_Printf(LT_INFO, "write: failed E=%d", pif_error);
				}
				else {
					pifLog_Printf(LT_INFO, "write: value = %Xh", value);
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

	if (argc == 1) {
		pifLog_Print(LT_INFO, "read [id] [size]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		id = atoi(argv[1]);
		size = atoi(argv[2]);
		p_data_info = (PifStorageFixDataInfo*)pifStorage_Open(&s_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_INFO, "read: not alloc ID=%d EC=%d", id, pif_error);
		}
		else {
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, 0, size);
				if (!pifStorage_Read(&s_storage.parent, p_buffer, (PifStorageDataInfoP)p_data_info, size)) {
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

static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = OFF;

	(void)pvIssuer;

	actLedL(sw);
	sw ^= 1;
}

void appSetup()
{
	PifTimer *pstTimer1ms;

	pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

	pifLog_Init();

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	if (!pifComm_AllocRxBuffer(&g_comm_log, 64)) return;
	if (!pifComm_AllocTxBuffer(&g_comm_log, 128)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);												// 500ms

	if (!pifStorageFix_Init(&s_storage, PIF_ID_AUTO)) return;
	if (!pifStorageFix_AttachActStorage(&s_storage, actStorageRead, actStorageWrite)) return;
	if (!pifStorageFix_SetMedia(&s_storage, STORAGE_SECTOR_SIZE, STORAGE_VOLUME)) return;

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
