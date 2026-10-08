// Do not remove the include below
#include "ex_obj_array.h"
#include "app_main.h"


#define TASK_SIZE				3

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
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
	int line;
	static PifUart s_uart_log;

	Serial.begin(UART_LOG_BAUDRATE);

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) {
		line = __LINE__;
		goto fail;
	}

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) {
		line = __LINE__;
		goto fail;
	}
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) {
		line = __LINE__;
		goto fail;
	}
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 1024)) {							// 1024bytes
		line = __LINE__;
		goto fail;
	}

	pifLog_Print(LT_NONE,"\n\n****************************************\n");
	pifLog_Print(LT_NONE,"***           ex_obj_array           ***\n");
	pifLog_Printf(LT_NONE,"***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE,"****************************************\n");

	if (!appSetup()) {
		line = __LINE__;
		goto fail;
	}

	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);
	return;

fail:	
	pifLog_Printf(LT_ERROR, "Error: %d Line=%d\n", pif_error, line);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
