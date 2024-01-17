// Do not remove the include below
#include "ex_modbus_rtu_slave.h"
#include "linker.h"


#define PIN_DIRECTION			10
#define PIN_LED_L				13

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			3

#define UART_LOG_BAUDRATE		115200
#define UART_MODBUS_BAUDRATE	9600

// The server address
#define RTU_SERVER_ADDRESS 		1

static PifUart s_uart_log;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static uint16_t actSlaveSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial1.write((char *)p_buffer, size);
}

static uint16_t actSlaveReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial1.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static uint8_t actSlaveGetTxRate(PifUart *p_uart)
{
	return 100 * Serial1.availableForWrite() / (SERIAL_BUFFER_SIZE - 1);
}

static void actSlaveDirection(PifUartDirection direction)
{
	digitalWrite(PIN_DIRECTION, direction);
}

static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_L, state & 1);
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
	int line;

	pinMode(PIN_DIRECTION, OUTPUT);
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE);
	Serial1.begin(UART_MODBUS_BAUDRATE);

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) { line = __LINE__; goto fail; }

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) { line = __LINE__; goto fail; }				// 1ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) { line = __LINE__; goto fail; }

	if (!pifUart_Init(&g_uart_modbus, PIF_ID_AUTO, UART_MODBUS_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTask(&g_uart_modbus, TM_PERIOD_MS, 1, "UartSlave")) { line = __LINE__; goto fail; }			// 1ms
    g_uart_modbus.act_receive_data = actSlaveReceiveData;
    g_uart_modbus.act_send_data = actSlaveSendData;
    g_uart_modbus.act_get_tx_rate = actSlaveGetTxRate;
    pifUart_AttachActDirection(&g_uart_modbus, actSlaveDirection, UD_RX);

	if (!pifModbusRtuSlave_Init(&g_modbus_slave, PIF_ID_AUTO, &g_timer_1ms, RTU_SERVER_ADDRESS, RTU_BAUDRATE)) { line = __LINE__; goto fail; }

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) { line = __LINE__; goto fail; }

	if (!appSetup()) { line = __LINE__; goto fail; }

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***       ex_modbus_rtu_slave        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return;

fail:
	pifLog_Printf(LT_INFO, "Initial failed. %d\n", line);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
