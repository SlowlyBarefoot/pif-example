// Do not remove the include below
#include "exTask5.h"

#include "core/pif_log.h"


#define PIN_LED_L				13

#define TASK_SIZE				5

#define UART_LOG_BAUDRATE		115200


static PifTask* s_task;
static int s_state = 0;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t _taskLedToggle(PifTask *pstTask)
{
	static int count = 0;
	static BOOL sw = LOW;

	(void)pstTask;

	if (count) {
		digitalWrite(PIN_LED_L, sw);
		sw ^= 1;

		count--;
	}
	else {
		switch (s_state) {
		case 0:
			pifTask_ChangeMode(s_task, TM_PERIOD_MS, 1);
			s_state++;
			break;

		case 1:
			pifTask_ChangeMode(s_task, TM_RATIO, 50);
			s_state = 0;
			break;
		}
		count = 100;
#ifdef PIF_DEBUG
		pifTaskManager_PrintRatioTable();
#endif
	}

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

	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;		// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

	s_task = pifTaskManager_Add(TM_RATIO, 50, _taskLedToggle, NULL, TRUE);		// 50%
	if (!s_task) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***              exTask5             ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());

#ifdef PIF_DEBUG
	pifTaskManager_PrintRatioTable();
#endif
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
