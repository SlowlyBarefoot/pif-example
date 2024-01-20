/**
 * 1초마다 log를 log buffer에 저장하다가 프로그램 종료시 모두 출력한다.
 *
 * Save log to log buffer every second and print them out at the end of the program.
 */
#include <stdio.h>
#include <time.h>

#include "appMain.h"
#include "timer.h"

#include "core/pif_log.h"


#define TASK_SIZE				1

#define UART_LOG_BAUDRATE		115200

#define LOG_BUFFER_SIZE			0x200


static volatile uint16_t s_unTimer = 10000;


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

	if (start_timer(1, &_TimerHandler)) {     										// 1ms
        printf("\nstart_timer error\n");
        return(1);
    }

    pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) goto fail;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) goto fail;	// 1ms
	s_uart_log.act_send_data = actLogSendData;

    if (!pifLog_InitHeap(LOG_BUFFER_SIZE)) goto fail;
	if (!pifLog_AttachUart(&s_uart_log)) goto fail;

    if (!appInit()) goto fail;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***           exLogBuffer            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);

    while (s_unTimer) {
    	pifTaskManager_Loop();
    }

    pifLog_PrintInBuffer();

fail:
	pifLog_Clear();
	pif_Exit();

    stop_timer();

	return pif_error;
}
