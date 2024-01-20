/**
 * 1초마다 log를 출력한다.
 *
 * Output log every second.
 */
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "appMain.h"
#include "timer.h"


#define TASK_SIZE				3
#define TIMER_MS_SIZE			4

#define UART_LOG_BAUDRATE		115200
#define UART_SERIAL_BAUDRATE	115200


static int s_fd;


static uint32_t micros()
{
	return 1000000L * clock() / CLOCKS_PER_SEC;
}

static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

	return fwrite((char *)pucBuffer, 1, usSize, stdout);
}

static uint16_t actSerialSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
    return write(s_fd, pucBuffer, usSize);
}

static uint16_t actSerialReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int rtn;

	rtn = read(s_fd, p_data, size);
	if (rtn == -1) return 0;
	return size;
}

static void _TimerHandler()
{
    pif_sigTimer1ms();

	pifTimerManager_sigTick(&g_timer_1ms);
}

int main(int argc, char **argv)
{
	int i;
    struct termios newtio;
    char port[16];
    static PifUart s_uart_log;

    if (start_timer(1, &_TimerHandler)) {     // 1ms
        printf("\nstart_timer error\n");
        return(1);
    }

    for (i = 1; i < 10; i++) {
    	sprintf(port, "/dev/ttyACM%d", i);
		s_fd = open( port, O_RDWR | O_NOCTTY );
		if (s_fd >= 0) break;
    }
	if (s_fd < 0) {
	    for (i = 0; i < 10; i++) {
	    	sprintf(port, "/dev/ttyUSB%d", i);
			s_fd = open( port, O_RDWR | O_NOCTTY );
			if (s_fd >= 0) break;
	    }
	}
	if (s_fd < 0) {
		fprintf(stderr, "All port open failed.\n");
		exit(-1);
	}
	else {
		printf("%s port opened.\n", port);
	}

    memset( &newtio, 0, sizeof(newtio) );

    newtio.c_cflag = B115200;
    newtio.c_cflag |= CS8;
    newtio.c_cflag |= CLOCAL;
    newtio.c_cflag |= CREAD;
    newtio.c_iflag = IGNPAR;
 //   newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;

    tcflush(s_fd, TCIFLUSH);
    tcsetattr(s_fd, TCSANOW, &newtio);

    pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_MS_SIZE)) goto fail;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) goto fail;			// 1ms
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) goto fail;

	if (!pifUart_Init(&s_serial, PIF_ID_AUTO, UART_SERIAL_BAUDRATE)) goto fail;
    if (!pifUart_AttachTask(&s_serial, TM_PERIOD_MS, 1, "UartSerial")) goto fail;			// 1ms
	s_serial.act_receive_data = actSerialReceiveData;
	s_serial.act_send_data = actSerialSendData;

    if (!appInit()) goto fail;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         exProtocolSerialM        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Pulse=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_MS_SIZE);

    while (1) {
    	pifTaskManager_Loop();
    }

fail:
	appExit();
	pifTimerManager_Clear(&g_timer_1ms);
	pifUart_Clear(&s_serial);
	pifUart_Clear(&s_uart_log);
	pifLog_Clear();
	pif_Exit();

    stop_timer();

	return pif_error;
}
