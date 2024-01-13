// Do not remove the include below
#include "ex_srml_host.h"
#include "linker.h"


#define PIN_LED_L				13
#define PIN_UART_CTS			4
#define PIN_UART_DSR			5

#define TASK_SIZE				6
#define TIMER_1MS_SIZE			2

#define UART_LOG_BAUDRATE		115200
#define UART_PRINTER_BAUDRATE	115200


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

static uint16_t actPrinterSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial1.write((char *)p_buffer, size);
}

static uint16_t actPrinterReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
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

static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_L, state & 1);
}

static void _isrUartCts()
{
	pifUart_SigTxFlowState(&g_uart_printer, !digitalRead(PIN_UART_CTS));
}

static void _isrUartDsr()
{
	pifUart_SigTxFlowState(&g_uart_printer, !digitalRead(PIN_UART_DSR));
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

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_UART_CTS, INPUT_PULLUP);
	pinMode(PIN_UART_DSR, INPUT_PULLUP);

	attachInterrupt(PIN_UART_CTS, _isrUartCts, CHANGE);
	attachInterrupt(PIN_UART_DSR, _isrUartDsr, CHANGE);

	Serial.begin(UART_LOG_BAUDRATE);
	Serial1.begin(UART_PRINTER_BAUDRATE);

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) { line = __LINE__; goto fail; }

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) { line = __LINE__; goto fail; }				// 1ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) { line = __LINE__; goto fail; }

	if (!pifUart_Init(&g_uart_printer, PIF_ID_AUTO, UART_PRINTER_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTask(&g_uart_printer, TM_PERIOD_MS, 1, "UartPrinter")) { line = __LINE__; goto fail; }			// 1ms
    g_uart_printer.act_receive_data = actPrinterReceiveData;
    g_uart_printer.act_send_data = actPrinterSendData;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) { line = __LINE__; goto fail; }

	if (!appSetup()) { line = __LINE__; goto fail; }

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***           ex_srml_host           ***\n");
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
