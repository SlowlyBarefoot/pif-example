// Do not remove the include below
#include "exDotMatrixS_SR.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_74HC595_DATA		48
#define PIN_74HC595_LATCH		50
#define PIN_74HC595_SHIFT		52

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			2

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actDotMatrixDisplay(uint8_t ucRow, uint8_t *pucData)
{
	digitalWrite(PIN_74HC595_LATCH, LOW);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, 1 << ucRow);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, ~(*pucData));
	digitalWrite(PIN_74HC595_LATCH, HIGH);
}

static uint32_t taskLedToggle(PifTask* p_task)
{
	static BOOL led = LOW;

	(void)p_task;

	digitalWrite(PIN_LED_L, led);
	led ^= 1;
	return 0;
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

	pinMode(PIN_74HC595_DATA, OUTPUT);
	pinMode(PIN_74HC595_LATCH, OUTPUT);
	pinMode(PIN_74HC595_SHIFT, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD, 1000, NULL)) return;					// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifDotMatrix_Init(&g_dot_matrix, PIF_ID_AUTO, &g_timer_1ms, 8, 8, actDotMatrixDisplay)) return;

    if (!pifTaskManager_Add(TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return;			// 500ms

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         exDotMatrixS_SR          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
