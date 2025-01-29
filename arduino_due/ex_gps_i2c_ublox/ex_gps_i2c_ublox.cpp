// Do not remove the include below
#include "ex_gps_i2c_ublox.h"
#include "linker.h"

#ifdef USE_I2C_WIRE
	#include <Wire.h>
#else
	#include "../i2c.h"
#endif


#define PIN_LED_L				13

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1

#define UART_LOG_BAUDRATE		115200


static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_L, state & 1);
}

static uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static PifI2cReturn actI2cWrite(PifI2cDevice *p_owner, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
#ifdef USE_I2C_WIRE
	int i;

	Wire1.beginTransmission(p_owner->addr);
	if (isize > 0) {
		for (i = isize - 1; i >= 0; i--) {
			Wire1.write((iaddr >> (i * 8)) & 0xFF);
		}
	}
    for (i = 0; i < (int)size; i++) {
    	Wire1.write(p_data[i]);
    }
    if (Wire1.endTransmission() != 0) {
		pif_error = E_TRANSFER_FAILED;
		return IR_ERROR;
	}
#else
	if (!I2C_WriteAddr(I2C_PORT_1, p_owner->addr, iaddr, isize, p_data, size)) return IR_ERROR;
#endif
    return IR_COMPLETE;
}

static PifI2cReturn actI2cRead(PifI2cDevice *p_owner, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
#ifdef USE_I2C_WIRE
	int i;
	uint8_t count;

	if (isize > 0) {
		Wire1.beginTransmission(p_owner->addr);
		for (i = isize - 1; i >= 0; i--) {
			Wire1.write((iaddr >> (i * 8)) & 0xFF);
		}
	    if (Wire1.endTransmission() != 0) {
			pif_error = E_TRANSFER_FAILED;
			return IR_ERROR;
		}
	}

    count = Wire1.requestFrom(p_owner->addr, (uint8_t)size);
    if (count < size) {
		pif_error = E_TRANSFER_FAILED;
		return IR_ERROR;
	}

    for (i = 0; i < (int)size; i++) {
    	p_data[i] = Wire1.read();
    }
#else
	if (!I2C_ReadAddr(I2C_PORT_1, p_owner->addr, iaddr, isize, p_data, size)) return IR_ERROR;
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

	Serial.begin(UART_LOG_BAUDRATE);

#ifdef USE_I2C_WIRE
	Wire1.begin();
//	Wire1.setClock(400000);
#else
	I2C_Init(I2C_PORT_1, I2C_CLOCK_100KHz);
//	I2C_Init(I2C_PORT_1, I2C_CLOCK_400KHz);
#endif

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 10, "UartLog")) return;				// 10ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;

    if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 1, NULL)) return;
    g_i2c_port.act_read = actI2cRead;
    g_i2c_port.act_write = actI2cWrite;

    if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         ex_gps_i2c_ublox         ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
