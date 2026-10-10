#include "app_main.h"


// 0x00-0x07 and 0x78-0x7F are reserved addresses (general call, CBUS, HS mode master codes,
// 10 bit addressing), so only 0x08-0x77 is probed, as i2cdetect does.
#define SCAN_FIRST_ADDR		0x08
#define SCAN_LAST_ADDR		0x77

#define SCAN_STEP_PERIOD	10000		// 10ms between two addresses (us)


PifI2cPort g_i2c_port;
PifTimerManager g_timer_1ms;


/**
 * @fn _probe
 * @brief Asks whether a device acknowledges an address, by a write with no data: only the slave
 *        address goes out, so nothing reaches a device as a register address or a command.
 * @details act_write is called directly rather than through pifI2cDevice_Write(), which counts
 *          and logs a failed transfer as an error. Here a missing device is the normal answer.
 *          It needs a port that makes the transfer before it returns, as the Wire port does.
 * @param addr Address to probe.
 * @return TRUE if a device acknowledged it.
 */
static BOOL _probe(uint8_t addr)
{
	PifI2cDevice* p_device = pifI2cPort_TemporaryDevice(&g_i2c_port, addr, NULL);

	if (!p_device) return FALSE;
	return (*g_i2c_port.act_write)(p_device, 0, 0, NULL, 0) == IR_COMPLETE;
}

/**
 * @fn _taskScan
 * @brief Probes one address each release, so the sweep does not hold the CPU, and reports what
 *        it found at the end. The task pauses itself then.
 * @param p_task Pointer to the task.
 * @return 0.
 */
static uint32_t _taskScan(PifTask* p_task)
{
	static uint8_t addr = SCAN_FIRST_ADDR;
	static int count = 0;

	if (_probe(addr)) {
		pifLog_Printf(LT_INFO, "I2C Addr:%Xh", addr);
		count++;
	}

	if (addr < SCAN_LAST_ADDR) {
		addr++;
		return 0;
	}

	if (count) {
		pifLog_Printf(LT_INFO, "I2C %d found", count);
	}
	else {
		pifLog_Print(LT_INFO, "I2C Not found");
	}
	p_task->pause = TRUE;
	return 0;
}

BOOL appSetup()
{
	if (!g_i2c_port.act_write) return FALSE;

	pifLog_Print(LT_INFO, "I2C Scan start");
	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, SCAN_STEP_PERIOD, _taskScan, NULL, TRUE)) return FALSE;
	return TRUE;
}
