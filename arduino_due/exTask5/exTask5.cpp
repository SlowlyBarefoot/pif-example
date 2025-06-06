// Do not remove the include below
#include "exTask5.h"

#include "core/pif_log.h"


#define PIN_PERIOD				12
#define PIN_LED_L				13

#define TASK_SIZE				4

#define UART_LOG_BAUDRATE		115200


static PifTask* s_task_period;
static PifTask* s_task_change;
static int s_state = 0;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint32_t _taskPeriod(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_PERIOD, sw);
	sw ^= 1;

	switch (s_state) {
	case 0:
		pifTask_ChangeMode(s_task_change, TM_EXTERNAL, 0);
		s_state = 1;
		break;

	case 1:
		pifTask_ChangeMode(s_task_change, TM_PERIOD, 1000);		// 1ms
		s_task_change->pause = FALSE;
		s_state = 0;
		break;
	}
	return 0;
}

static uint32_t _taskChange(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;

    return 0;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_PERIOD, OUTPUT);
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return;												// 256bytes

	s_task_period = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, _taskPeriod, NULL, TRUE);	// 500ms
	if (!s_task_period) return;

	s_task_change = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskChange, NULL, TRUE);		// 1ms
	if (!s_task_change) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***              exTask5             ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);

#ifdef PIF_DEBUG
	pifTaskManager_PrintRatioTable();
#endif
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
