#define _DEFAULT_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "host.h"
#include "app_port.h"


#define UART_LOG_BAUDRATE		115200
#define UART_SERIAL_BAUDRATE	115200


static int s_fd = -1;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return hostConsoleWrite(pucBuffer, usSize);
}

// The LED is not drawn here: drawing it as the other examples do would break into the log lines.
static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;
	(void)unState;
}

static uint16_t actXmodemSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	ssize_t length;

	(void)p_uart;

	length = write(s_fd, pucBuffer, usSize);
	return length > 0 ? (uint16_t)length : 0;
}

static uint16_t actXmodemReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	ssize_t length, i;

	(void)p_uart;

	// VMIN and VTIME are 0, so the read does not wait.
	length = read(s_fd, p_data, size);
	if (length <= 0) return 0;
	for (i = 0; i < length; i++) {
		if (p_data[i] == 'C') hostConsoleWrite((const uint8_t *)"C", 1);
	}
	return (uint16_t)length;
}

static BOOL _openSerial()
{
	struct termios newtio;
	char port[16];
	int i;

	for (i = 1; i < 10 && s_fd < 0; i++) {
		sprintf(port, "/dev/ttyACM%d", i);
		s_fd = open(port, O_RDWR | O_NOCTTY);
	}
	for (i = 0; i < 10 && s_fd < 0; i++) {
		sprintf(port, "/dev/ttyUSB%d", i);
		s_fd = open(port, O_RDWR | O_NOCTTY);
	}
	if (s_fd < 0) {
		fprintf(stderr, "All port open failed.\n");
		return FALSE;
	}
	printf("%s port opened.\n", port);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;
	cfsetispeed(&newtio, B115200);
	cfsetospeed(&newtio, B115200);

	tcflush(s_fd, TCIFLUSH);
	tcsetattr(s_fd, TCSANOW, &newtio);
	return TRUE;
}

// Body of the SysTick handler of a board, called from the tick thread of host.c.
void portTick1ms()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	if (!_openSerial()) return FALSE;

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;
	pifTaskManager_SetIdle(hostEvtIdle, 0);

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartLog")) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;												// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;

	if (!pifUart_Init(&g_serial, PIF_ID_AUTO, UART_SERIAL_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&g_serial, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxSerial")) return FALSE;
    if (!pifUart_AttachRxTask(&g_serial, PIF_ID_AUTO, TM_PERIOD, 50000, "UartRxSerial")) return FALSE;	// 50ms
    g_serial.act_receive_data = actXmodemReceiveData;
    g_serial.act_send_data = actXmodemSendData;
	return TRUE;
}
