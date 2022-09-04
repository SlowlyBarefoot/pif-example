#include "app_main.h"
#include "main.h"

#include "pif_storage_var.h"


#define ATMEL_I2C_ADDRESS		0x50

#define EEPROM_AT24C08
//#define EEPROM_AT24C256

#define AT24C08_SECTOR_SIZE		16
#define AT24C08_VOLUME			1024

#define AT24C256_SECTOR_SIZE	64
#define AT24C256_VOLUME			32768


PifComm g_comm_log;
PifTimerManager g_timer_1ms;

static PifI2cPort s_i2c_port;
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

	pifStorage_Format(&s_storage.parent);
	pifLog_Print(LT_INFO, "Storage format\n");
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdCreate(int argc, char *argv[])
{
	uint16_t id, size;

	if (argc == 1) {
		pifLog_Print(LT_NONE, "create [id] [size]\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		id = atoi(argv[1]);
		size = atoi(argv[2]);
		if (!pifStorage_Create(&s_storage.parent, id, size)) {
			pifLog_Printf(LT_NONE, "create: failed E=%d\n", pif_error);
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
		pifLog_Print(LT_NONE, "delete [id]\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		if (!pifStorage_Delete(&s_storage.parent, id)) {
			pifLog_Printf(LT_NONE, "delete: failed E=%d\n", pif_error);
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
		pifLog_Print(LT_NONE, "write [id] [value]\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		p_data_info = (PifStorageVarDataInfo*)pifStorage_Open(&s_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_NONE, "write: not alloc ID=%d E=%d\n", id, pif_error);
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
				if (!pifStorage_Write(&s_storage.parent, (PifStorageDataInfo*)p_data_info, p_buffer, p_data_info->size)) {
					pifLog_Printf(LT_NONE, "write: failed E=%d\n", pif_error);
				}
				else {
					pifLog_Printf(LT_NONE, "write: value = %Xh\n", value);
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
		pifLog_Print(LT_NONE, "read [id]\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		p_data_info = (PifStorageVarDataInfo*)pifStorage_Open(&s_storage.parent, id);
		if (!p_data_info) {
			pifLog_Printf(LT_NONE, "read: not alloc ID=%d EC=%d\n", id, pif_error);
		}
		else {
			size = p_data_info->size;
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, 0, size);
				if (!pifStorage_Read(&s_storage.parent, p_buffer, (PifStorageDataInfo*)p_data_info, p_data_info->size)) {
					pifLog_Printf(LT_NONE, "read: failed E=%d\n", pif_error);
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

	pifStorageVar_Dump(&s_storage, 0, AT24C08_VOLUME);
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

    if (!pifI2cPort_Init(&s_i2c_port, PIF_ID_AUTO, 1, 16)) return;
    s_i2c_port.act_read = actI2cRead;
    s_i2c_port.act_write = actI2cWrite;

	if (!pifStorageVar_Init(&s_storage, PIF_ID_AUTO)) return;
#ifdef EEPROM_AT24C08
	if (!pifStorageVar_AttachI2c(&s_storage, &s_i2c_port, ATMEL_I2C_ADDRESS, SIC_I_ADDR_SIZE_1, 5)) return;	// 5ms
	if (!pifStorageVar_SetMedia(&s_storage, AT24C08_SECTOR_SIZE, AT24C08_VOLUME, 5)) return;
#endif
#ifdef EEPROM_AT24C256
	if (!pifStorageVar_AttachI2c(&s_storage, &s_i2c_port, ATMEL_I2C_ADDRESS, SIC_I_ADDR_SIZE_2, 5)) return;	// 5ms
	if (!pifStorageVar_SetMedia(&s_storage, AT24C256_SECTOR_SIZE, AT24C256_VOLUME, 10)) return;
#endif
	if (!pifStorageVar_IsFormat(&s_storage.parent)) {
		pifLog_Printf(LT_INFO, "Storage Init : EC=%d\n", pif_error);
		if (!pifStorage_Format(&s_storage.parent)) {
			pifLog_Printf(LT_INFO, "Storage format failed\n");
		}
		else {
			pifLog_Printf(LT_INFO, "Storage format\n");
		}
	}

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));

#ifdef __PIF_DEBUG__
	if (!pifStorageVar_IsFormat(&s_storage.parent)) {
		pifStorageVar_PrintInfo(&s_storage, TRUE);
	}
#endif
}
