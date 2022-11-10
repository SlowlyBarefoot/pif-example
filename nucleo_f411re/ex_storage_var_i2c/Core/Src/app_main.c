#include "app_main.h"
#include "main.h"

#include "core/pif_log.h"
#include "storage/pif_storage_var.h"


#define ATMEL_I2C_ADDRESS		0x50

//#define EEPROM_AT24C08
#define EEPROM_AT24C256

#ifdef EEPROM_AT24C08
	#define EEPROM_PAGE_SIZE		16
	#define EEPROM_SECTOR_SIZE		16
	#define EEPROM_VOLUME			1024
	#define EEPROM_I_ADDR_SIZE		SIC_I_ADDR_SIZE_1
	#define MIN_DATA_INFO_COUNT		5
#endif

#ifdef EEPROM_AT24C256
	#define EEPROM_PAGE_SIZE		64
	#define EEPROM_SECTOR_SIZE		64
	#define EEPROM_VOLUME			32768
	#define EEPROM_I_ADDR_SIZE		SIC_I_ADDR_SIZE_2
	#define MIN_DATA_INFO_COUNT		8
#endif


PifComm g_comm_log;
PifI2cPort g_i2c_port;
PifTimerManager g_timer_1ms;

static PifStorageVar s_storage;

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
		{ "format", _CmdFormat, "Storage format" },
		{ "create", _CmdCreate, "Storage create" },
		{ "delete", _CmdDelete, "Storage delete" },
		{ "write", _CmdWrite, "Storage write" },
		{ "read", _CmdRead, "Storage read" },
#ifdef __PIF_DEBUG__
		{ "info", _CmdPrintInfo, "Storage print info" },
		{ "dump", _CmdDump, "Storage dump" },
#endif
		{ NULL, NULL, NULL }
};


static int _CmdFormat(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	if (pifStorage_Format(&s_storage.parent)) {
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

	if (argc == 1) {
		pifLog_Print(LT_INFO, "create [id] [size]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		id = atoi(argv[1]);
		size = atoi(argv[2]);
		if (!pifStorage_Create(&s_storage.parent, id, size)) {
			pifLog_Printf(LT_INFO, "create: failed E=%d", pif_error);
		}
#ifdef __PIF_DEBUG__
		else {
			pifStorageVar_PrintInfo(&s_storage, TRUE);
		}
#endif
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdDelete(int argc, char *argv[])
{
	uint16_t id;

	if (argc == 1) {
		pifLog_Print(LT_INFO, "delete [id]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		if (!pifStorage_Delete(&s_storage.parent, id)) {
			pifLog_Printf(LT_INFO, "delete: failed E=%d", pif_error);
		}
#ifdef __PIF_DEBUG__
		else {
			pifStorageVar_PrintInfo(&s_storage, TRUE);
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

	if (argc == 1) {
		pifLog_Print(LT_INFO, "write [id] [value]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		p_data_info = (PifStorageVarDataInfo*)pifStorage_Open(&s_storage.parent, id);
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
			p_buffer = (uint8_t*)malloc(p_data_info->size);
			if (p_buffer) {
				memset(p_buffer, value, p_data_info->size);
				if (!pifStorage_Write(&s_storage.parent, (PifStorageDataInfoP)p_data_info, p_buffer, p_data_info->size)) {
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
	PifStorageVarDataInfo* p_data_info;

	if (argc == 1) {
		pifLog_Print(LT_INFO, "read [id]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		p_data_info = (PifStorageVarDataInfo*)pifStorage_Open(&s_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_INFO, "read: not alloc ID=%d EC=%d", id, pif_error);
		}
		else {
			size = p_data_info->size;
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, 0, size);
				if (!pifStorage_Read(&s_storage.parent, p_buffer, (PifStorageDataInfoP)p_data_info, p_data_info->size)) {
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
	(void)argc;
	(void)argv;

	if (argc == 1) {
		pifStorageVar_PrintInfo(&s_storage, FALSE);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (argv[1][0] == 'h') {
			pifStorageVar_PrintInfo(&s_storage, TRUE);
			return PIF_LOG_CMD_NO_ERROR;
		}
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdDump(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pifStorageVar_Dump(&s_storage, 0, EEPROM_VOLUME);
	return PIF_LOG_CMD_NO_ERROR;
}

#endif

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

    if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 1, EEPROM_PAGE_SIZE)) return;
    g_i2c_port.act_read = actI2cRead;
    g_i2c_port.act_write = actI2cWrite;

	if (!pifStorageVar_Init(&s_storage, PIF_ID_AUTO)) return;
	if (!pifStorageVar_AttachI2c(&s_storage, &g_i2c_port, ATMEL_I2C_ADDRESS, EEPROM_I_ADDR_SIZE, 10)) return;	// 10ms
	if (!pifStorageVar_SetMedia(&s_storage, EEPROM_SECTOR_SIZE, EEPROM_VOLUME, MIN_DATA_INFO_COUNT)) return;
	if (!pifStorageVar_IsFormat(&s_storage.parent)) {
		pifLog_Printf(LT_INFO, "Storage Init : EC=%d", pif_error);
		if (!pifStorage_Format(&s_storage.parent)) {
			pifLog_Printf(LT_INFO, "Storage format failed");
		}
		else {
			pifLog_Printf(LT_INFO, "Storage format");
		}
	}

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));

#ifdef __PIF_DEBUG__
	if (!pifStorageVar_IsFormat(&s_storage.parent)) {
		pifStorageVar_PrintInfo(&s_storage, TRUE);
	}
#endif
}
