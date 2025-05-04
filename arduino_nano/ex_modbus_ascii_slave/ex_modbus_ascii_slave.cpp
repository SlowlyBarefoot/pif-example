// Do not remove the include below
#include "ex_modbus_ascii_slave.h"
#include "linker.h"

#include <MsTimer2.h>


#define PIN_DIRECTION			10
#define PIN_LED_L				13

#define TASK_SIZE				7
#define TIMER_1MS_SIZE			3

//#define UART_MODBUS_BAUDRATE	9600
#define UART_MODBUS_BAUDRATE	115200

// The server address
#define RTU_SERVER_ADDRESS 		2


static uint16_t actSlaveSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static uint16_t actSlaveReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
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

static uint8_t actSlaveGetTxRate(PifUart *p_uart)
{
	return 100 * Serial.availableForWrite() / (SERIAL_TX_BUFFER_SIZE - 1);
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
void setup()
{
	int line;

	pinMode(PIN_DIRECTION, OUTPUT);
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_MODBUS_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&g_uart_modbus, PIF_ID_AUTO, UART_MODBUS_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&g_uart_modbus, TM_EXTERNAL_ORDER, 0, NULL)) return;
    if (!pifUart_AttachRxTask(&g_uart_modbus, TM_PERIOD, 5000, NULL)) return;				// 5ms
    g_uart_modbus.act_receive_data = actSlaveReceiveData;
    g_uart_modbus.act_send_data = actSlaveSendData;
    g_uart_modbus.act_get_tx_rate = actSlaveGetTxRate;
    pifUart_AttachActDirection(&g_uart_modbus, actSlaveDirection, UD_RX);

	if (!pifModbusAsciiSlave_Init(&g_modbus_slave, PIF_ID_AUTO, &g_timer_1ms, RTU_SERVER_ADDRESS)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

	if (!appSetup()) return;
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
