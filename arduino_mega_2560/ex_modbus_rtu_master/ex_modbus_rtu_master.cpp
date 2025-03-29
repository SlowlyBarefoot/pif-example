// Do not remove the include below
#include "ex_modbus_rtu_master.h"
#include "linker.h"

#include <MsTimer2.h>


#define PIN_DIRECTION			10
#define PIN_LED_L				13

#define TASK_SIZE				10
#define TIMER_1MS_SIZE			5

#define UART_LOG_BAUDRATE		115200
//#define UART_MODBUS_BAUDRATE	9600
#define UART_MODBUS_BAUDRATE	115200


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

static uint16_t actMasterSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial1.write((char *)p_buffer, size);
}

static uint16_t actMasterReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
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

static uint8_t actMasterGetTxRate(PifUart *p_uart)
{
	return 100 * Serial1.availableForWrite() / (SERIAL_TX_BUFFER_SIZE - 1);
}

static void actMasterDirection(PifUartDirection direction)
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

	Serial.begin(UART_LOG_BAUDRATE);
	Serial1.begin(UART_MODBUS_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) { line = __LINE__; goto fail; }

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD, 1000, "UartLog")) { line = __LINE__; goto fail; }				// 1ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) { line = __LINE__; goto fail; }

	if (!pifUart_Init(&g_uart_modbus, PIF_ID_AUTO, UART_MODBUS_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTask(&g_uart_modbus, TM_PERIOD, 500, "UartModbus")) { line = __LINE__; goto fail; }			// 500us
    g_uart_modbus.act_receive_data = actMasterReceiveData;
    g_uart_modbus.act_send_data = actMasterSendData;
    g_uart_modbus.act_get_tx_rate = actMasterGetTxRate;
    pifUart_AttachActDirection(&g_uart_modbus, actMasterDirection, UD_TX);

	if (!pifModbusRtuMaster_Init(&g_modbus_master, PIF_ID_AUTO, &g_timer_1ms)) { line = __LINE__; goto fail; }

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) { line = __LINE__; goto fail; }

	if (!appSetup()) { line = __LINE__; goto fail; }

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***       ex_modbus_rtu_master       ***\n");
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
