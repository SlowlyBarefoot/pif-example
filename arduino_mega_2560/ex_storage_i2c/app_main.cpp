#include "app_main.h"
#include "ex_storage_i2c.h"

#include "pif_log.h"
#include "pif_eeprom_i2c.h"


#define ATMEL_I2C_ADDRESS		0x50

//#define EEPROM_AT24C08
#define EEPROM_AT24C256

#define AT24C08_SECTOR_SIZE		16
#define AT24C08_VOLUME			1024

#define AT24C256_SECTOR_SIZE	64
#define AT24C256_VOLUME			32768


PifTimerManager g_timer_1ms;

static PifI2cPort s_i2c_port;
static PifEepromI2c s_eeprom_i2c;

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
		{ "format", _CmdFormat, "Storage format" },
		{ "alloc", _CmdAlloc, "Storage alloc" },
		{ "free", _CmdFree, "Storage free" },
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

	pifStorage_Format(&s_eeprom_i2c._storage);
	pifLog_Print(LT_INFO, "Storage format\n");
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdAlloc(int argc, char *argv[])
{
	uint16_t id, size;

	if (argc == 1) {
		pifLog_Print(LT_NONE, "alloc [id] [size]\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		id = atoi(argv[1]);
		size = atoi(argv[2]);
		if (!pifStorage_Alloc(&s_eeprom_i2c._storage, id, size)) {
			pifLog_Printf(LT_NONE, "alloc: failed E=%d\n", pif_error);
		}
#ifdef __PIF_DEBUG__
		else {
			pifStorage_PrintInfo(&s_eeprom_i2c._storage, TRUE);
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
		pifLog_Print(LT_NONE, "free [id]\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		if (!pifStorage_Free(&s_eeprom_i2c._storage, id)) {
			pifLog_Printf(LT_NONE, "free: failed E=%d\n", pif_error);
		}
#ifdef __PIF_DEBUG__
		else {
			pifStorage_PrintInfo(&s_eeprom_i2c._storage, TRUE);
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
		pifLog_Print(LT_NONE, "write [id]\n");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		p_data_info = pifStorage_GetDataInfo(&s_eeprom_i2c._storage, id);
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
				if (!pifStorage_Write(&s_eeprom_i2c._storage, p_data_info, p_buffer)) {
					pifLog_Printf(LT_NONE, "write: failed E=%d\n", pif_error);
				}
				else {
					pifLog_Printf(LT_NONE, "write: value = %Xh\n", value);
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
		pifLog_Print(LT_NONE, "\nread [id]");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		id = atoi(argv[1]);
		p_data_info = pifStorage_GetDataInfo(&s_eeprom_i2c._storage, id);
		if (!p_data_info) {
			pifLog_Printf(LT_NONE, "read: not alloc ID=%d EC=%d\n", id, pif_error);
		}
		else {
			size = p_data_info->size;
			p_buffer = (uint8_t*)malloc(size);
			if (p_buffer) {
				memset(p_buffer, 0, size);
				if (!pifStorage_Read(&s_eeprom_i2c._storage, p_data_info, p_buffer)) {
					pifLog_Printf(LT_NONE, "read: failed E=%d\n", pif_error);
				}
				else {
					for (i = 0; i < size; i++) {
						if (!(i % 16)) {
							pifLog_Printf(LT_NONE, "%04X: ", p_data_info->first_sector * s_eeprom_i2c._storage._p_info->sector_size + i);
						}
						pifLog_Printf(LT_NONE, "%02X ", p_buffer[i]);
					}
					pifLog_Print(LT_NONE, "\n");
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
		pifStorage_PrintInfo(&s_eeprom_i2c._storage, FALSE);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (argv[1][0] == 'h') {
			pifStorage_PrintInfo(&s_eeprom_i2c._storage, TRUE);
			return PIF_LOG_CMD_NO_ERROR;
		}
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdDump(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pifStorage_Dump(&s_eeprom_i2c._storage, 0, AT24C08_VOLUME);
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
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;
	s_comm_log.act_receive_data = actLogReceiveData;

	if (!pifLog_AttachComm(&s_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

    pstTimer1ms = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimer1ms) return;
    pifTimer_AttachEvtFinish(pstTimer1ms, evtLedToggle, NULL);
    pifTimer_Start(pstTimer1ms, 500);												// 500ms

    if (!pifI2cPort_Init(&s_i2c_port, PIF_ID_AUTO, 1, 16)) return;
    s_i2c_port.act_read = actI2cRead;
    s_i2c_port.act_write = actI2cWrite;

#ifdef EEPROM_AT24C08
	if (!pifEepromI2c_Init(&s_eeprom_i2c, PIF_ID_AUTO, 5, AT24C08_SECTOR_SIZE, AT24C08_VOLUME,
			&s_i2c_port, ATMEL_I2C_ADDRESS, EEPROM_I2C_I_ADDR_SIZE_1)) return;
#endif
#ifdef EEPROM_AT24C256
	if (!pifEepromI2c_Init(&s_eeprom_i2c, PIF_ID_AUTO, 10, AT24C256_SECTOR_SIZE, AT24C256_VOLUME,
			&s_i2c_port, ATMEL_I2C_ADDRESS, EEPROM_I2C_I_ADDR_SIZE_2, 5)) return;	// 5ms
#endif
	if (!s_eeprom_i2c._storage._is_format) {
		pifLog_Printf(LT_INFO, "Storage Init : EC=%d\n", pif_error);
		if (!pifStorage_Format(&s_eeprom_i2c._storage)) {
			pifLog_Printf(LT_INFO, "Storage format failed\n");
		}
		else {
			pifLog_Printf(LT_INFO, "Storage format\n");
		}
	}

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));

#ifdef __PIF_DEBUG__
	if (s_eeprom_i2c._storage._is_format) {
		pifStorage_PrintInfo(&s_eeprom_i2c._storage, TRUE);
	}
#endif
}
