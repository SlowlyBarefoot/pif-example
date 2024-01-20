/**
 * 1초마다 log를 출력한다.
 *
 * Output log every second.
 */
#include <stdio.h>
#include <time.h>

#include "timer.h"

#include "core/pif_log.h"


#define TASK_SIZE				1

#define UART_LOG_BAUDRATE		115200


static volatile uint16_t s_unTimer = 30000;


static uint32_t micros()
{
	return 1000000L * clock() / CLOCKS_PER_SEC;
}

static void _TimerHandler()
{
	static int nTimer = 0;

    if (s_unTimer) s_unTimer--;

    pif_sigTimer1ms();

    if (nTimer) nTimer--;
    else {
    	pifLog_Printf(LT_INFO, "1sec Timer");
    	nTimer = 1000 - 1;
    }
}

static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return fwrite((char *)pucBuffer, 1, usSize, stdout);
}

int main(int argc, char **argv)
{
	static PifUart s_uart_log;

	if (start_timer(1, &_TimerHandler)) {  											// 1ms
        printf("\nstart_timer error\n");
        return(1);
    }

    pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) goto fail;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) goto fail;	// 1ms
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) goto fail;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            exLogPrint            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);

    while (s_unTimer) {
    	pifTaskManager_Loop();
    }

fail:
	pifLog_Clear();
	pif_Exit();

    stop_timer();

	return pif_error;
}
