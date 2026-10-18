#include "ex_modbus_ascii_slave.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_DIRECTION			10
#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200
//#define UART_MODBUS_BAUDRATE	9600
#define UART_MODBUS_BAUDRATE	115200

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
	return 100 * Serial1.availableForWrite() / (SERIAL_TX_BUFFER_SIZE - 1);
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

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
BOOL portSetup()
{
	int line;

	pinMode(PIN_DIRECTION, OUTPUT);
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);
	Serial1.begin(UART_MODBUS_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) { line = __LINE__; goto fail; }

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }					// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) { line = __LINE__; goto fail; }
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) { line = __LINE__; goto fail; }		// 200ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) { line = __LINE__; goto fail; }													// 256bytes

	if (!pifUart_Init(&g_uart_modbus, PIF_ID_AUTO, UART_MODBUS_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTxTask(&g_uart_modbus, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxModbus")) { line = __LINE__; goto fail; }
    if (!pifUart_AttachRxTask(&g_uart_modbus, PIF_ID_AUTO, TM_PERIOD, 5000, "UartRxModbus")) { line = __LINE__; goto fail; }	// 5ms
    g_uart_modbus.act_receive_data = actSlaveReceiveData;
    g_uart_modbus.act_send_data = actSlaveSendData;
    g_uart_modbus.act_get_tx_rate = actSlaveGetTxRate;
    pifUart_AttachActDirection(&g_uart_modbus, actSlaveDirection, UD_RX);

	if (!pifModbusAsciiSlave_Init(&g_modbus_slave, PIF_ID_AUTO, RTU_SERVER_ADDRESS)) { line = __LINE__; goto fail; }

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) { line = __LINE__; goto fail; }
	return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Initial failed. %d\n", line);
	return FALSE;
}
