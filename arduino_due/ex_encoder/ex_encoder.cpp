// Do not remove the include below
#include "ex_encoder.h"
#include "app_main.h"


// Encoder input. With USE_GENERATOR connect PIN_GEN_A to PIN_ENCODER_A and PIN_GEN_B to PIN_ENCODER_B.
// Otherwise connect phase A and B of an encoder.
#define PIN_LED_L				13

#define PIN_ENCODER_A			2
#define PIN_ENCODER_B			3

#define PIN_GEN_A				4
#define PIN_GEN_B				5

#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1

#define UART_LOG_BAUDRATE		115200


static uint32_t _taskLedToggle(PifTask* p_task)
{
	static BOOL sw = LOW;

	(void)p_task;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
    return 0;
}

static uint16_t actLogSendData(PifUart* p_uart, uint8_t* p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static void _actGenerate(uint8_t state)
{
	digitalWrite(PIN_GEN_A, (state & ENCODER_PHASE_A) ? HIGH : LOW);
	digitalWrite(PIN_GEN_B, (state & ENCODER_PHASE_B) ? HIGH : LOW);
}

static void _isrEncoder()
{
	uint8_t state = (digitalRead(PIN_ENCODER_A) ? ENCODER_PHASE_A : 0) | (digitalRead(PIN_ENCODER_B) ? ENCODER_PHASE_B : 0);

	pifEncoder_sigState(&g_encoder, state, micros());
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
	pinMode(PIN_ENCODER_A, INPUT_PULLUP);
	pinMode(PIN_ENCODER_B, INPUT_PULLUP);
#if USE_GENERATOR
	pinMode(PIN_GEN_A, OUTPUT);
	pinMode(PIN_GEN_B, OUTPUT);
#endif

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;				// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return;
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return;												// 256bytes

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 100000, _taskLedToggle, NULL, TRUE)) return;	// 100ms

	if (!appSetup(_actGenerate)) return;

	// The first state only sets where the encoder stands. Every edge of either phase after it is a step.
	delayMicroseconds(10);
	_isrEncoder();
	attachInterrupt(PIN_ENCODER_A, _isrEncoder, CHANGE);
	attachInterrupt(PIN_ENCODER_B, _isrEncoder, CHANGE);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            ex_encoder            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
