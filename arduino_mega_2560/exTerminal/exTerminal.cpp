#include "exTerminal.h"
#include "appMain.h"

#define USE_SERIAL
//#define USE_USART

// Do not remove the include below
#include <MsTimer2.h>

#ifdef USE_USART
#include "../usart.h"
#endif


#define PIN_LED_L				13

#define TASK_SIZE				3
#define TIMER_1MS_SIZE			1

#define UART_LOG_BAUDRATE		115200


static PifUart s_uart_log;


#ifdef USE_SERIAL

static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *pucData, uint16_t size)
{
	int rxData;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		rxData = Serial.read();
		if (rxData < 0) break;
		pucData[i] = rxData;
	}
	return i;
}

#endif

#ifdef USE_USART

static BOOL actLogStartTransfer(PifUart* p_uart)
{
	(void)p_uart;

	return USART_StartTransfer(0);
}

ISR(USART0_UDRE_vect)
{
	USART_Send(0, &s_uart_log);
}

ISR(USART0_RX_vect)
{
	USART_Receive(0, &s_uart_log);
}

#endif

static void actLedLState(PifId usPifId, uint32_t unState)
{
	static uint32_t state;

	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
	if (unState != state) {
		pifLog_Printf(LT_INFO, "LED State=%u", unState);
		state = unState;
	}
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

#ifdef USE_SERIAL
	Serial.begin(UART_LOG_BAUDRATE);
#endif
#ifdef USE_USART
	USART_Init(0, UART_LOG_BAUDRATE, DATA_BIT_DEFAULT | PARITY_DEFAULT | STOP_BIT_DEFAULT, TRUE);

	// Enable Global Interrupts
	sei();
#endif

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
#ifdef USE_SERIAL
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;
#endif
#ifdef USE_USART
	if (!pifUart_AllocRxBuffer(&s_uart_log, 64, 100)) return;								// 100%
	if (!pifUart_AllocTxBuffer(&s_uart_log, 64)) return;
	s_uart_log.act_start_transfer = actLogStartTransfer;
#endif

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
