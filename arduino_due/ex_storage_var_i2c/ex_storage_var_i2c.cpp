// Do not remove the include below
#include "ex_storage_var_i2c.h"
#include "app_main.h"

//#define USE_I2C_WIRE

#ifdef USE_I2C_WIRE
	#include <Wire.h>
#else
	#include "../i2c.h"
#endif


#define PIN_LED_L				13

#define TASK_SIZE				3
#define TIMER_1MS_SIZE			1

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
	#define MIN_DATA_INFO_COUNT		10
#endif


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size, uint8_t* p_rate)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	if (p_rate) *p_rate = 100 * Serial.available() / SERIAL_BUFFER_SIZE;
	return i;
}

static void evtLedToggle(void* p_issuer)
{
	static BOOL sw = OFF;

	(void)p_issuer;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

static PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
#ifdef USE_I2C_WIRE
	int i;
	uint8_t count;
	uint16_t n;

	if (isize > 0) {
		Wire.beginTransmission(addr);
		for (i = isize - 1; i >= 0; i--) {
			Wire.write((iaddr >> (i * 8)) & 0xFF);
		}
	    if (Wire.endTransmission() != 0) {
			pif_error = E_TRANSFER_FAILED;
			return IR_ERROR;
		}
	}

    count = Wire.requestFrom(addr, (uint8_t)size);
    if (count < size) {
		pif_error = E_TRANSFER_FAILED;
		return IR_ERROR;
	}

    for (n = 0; n < size; n++) {
    	p_data[n] = Wire.read();
    }
#else
	if (!I2C_ReadAddr(I2C_PORT_0, addr, iaddr, isize, p_data, size)) return IR_ERROR;
#endif
    return IR_COMPLETE;
}

static PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
#ifdef USE_I2C_WIRE
	int i;
	uint16_t n;

	Wire.beginTransmission(addr);
	if (isize > 0) {
		for (i = isize - 1; i >= 0; i--) {
			Wire.write((iaddr >> (i * 8)) & 0xFF);
		}
	}
    for (n = 0; n < size; n++) {
    	Wire.write(p_data[n]);
    }
    if (Wire.endTransmission() != 0) {
		pif_error = E_TRANSFER_FAILED;
		return IR_ERROR;
	}
#else
	if (!I2C_WriteAddr(I2C_PORT_0, addr, iaddr, isize, p_data, size)) return IR_ERROR;
#endif
    return IR_COMPLETE;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);

#ifdef USE_I2C_WIRE
	Wire.begin();
#else
	I2C_Init(I2C_PORT_0, I2C_CLOCK_100KHz);
#endif

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;							// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;										// 1ms
	s_uart_log.act_send_data = actLogSendData;
	s_uart_log.act_receive_data = actLogReceiveData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_led) return;
    pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL);

    if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 1, EEPROM_PAGE_SIZE)) return;
    g_i2c_port.act_read = actI2cRead;
    g_i2c_port.act_write = actI2cWrite;

	if (!pifStorageVar_Init(&g_storage, PIF_ID_AUTO)) return;
	if (!pifStorageVar_AttachI2c(&g_storage, &g_i2c_port, ATMEL_I2C_ADDRESS, EEPROM_I_ADDR_SIZE, 10)) return;	// 10ms
	if (!pifStorageVar_SetMedia(&g_storage, EEPROM_SECTOR_SIZE, EEPROM_VOLUME, MIN_DATA_INFO_COUNT)) return;

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
