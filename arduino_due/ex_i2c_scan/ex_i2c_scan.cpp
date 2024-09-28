// Do not remove the include below
#include "ex_i2c_scan.h"
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

#define UART_LOG_BAUDRATE			115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

static PifI2cReturn actI2cWrite(PifI2cDevice *p_owner, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
#ifdef USE_I2C_WIRE
	int i;

	Wire.beginTransmission(p_owner->addr);
	if (isize > 0) {
		for (i = isize - 1; i >= 0; i--) {
			Wire.write((iaddr >> (i * 8)) & 0xFF);
		}
	}
    for (i = 0; i < size; i++) {
    	Wire.write(p_data[i]);
    }
    if (Wire.endTransmission() != 0) {
		pif_error = E_TRANSFER_FAILED;
		return IR_ERROR;
	}
#else
	if (!I2C_WriteAddr(I2C_PORT_0, p_owner->addr, iaddr, isize, p_data, size)) return IR_ERROR;
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
	Wire.begin();
#else
	I2C_Init(I2C_PORT_0, I2C_CLOCK_100KHz);
#endif

    pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 1, 16, NULL)) return;
    g_i2c_port.act_write = actI2cWrite;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***           ex_i2c_scan            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);

    if (!appSetup()) return;
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
