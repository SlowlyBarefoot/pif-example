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

#include "main.h"
#include "appMain.h"
#include "timer.h"


static int s_fd;


void actLogPrint(char *pcString)
{
	printf("%s", pcString);
}

BOOL actSerialSendData(PIF_stRingBuffer *pstBuffer)
{
	uint8_t txData;

    while (pifRingBuffer_GetByte(pstBuffer, &txData)) {
        write(s_fd, &txData, 1);
    }
    return TRUE;
}

void actSerialReceiveData(PIF_stRingBuffer *pstBuffer)
{
	uint16_t size = pifRingBuffer_GetRemainSize(pstBuffer);
	uint8_t rxData[8];

	if (size) {
		size = read(s_fd, rxData, size > 8 ? 8 : size);
		if (size) {
			pifRingBuffer_PutData(pstBuffer, rxData, size);
		}
	}
}

static void _TimerHandler()
{
    pif_sigTimer1ms();

	pifPulse_sigTick(g_pstTimer1ms);
}

int main(int argc, char **argv)
{
	int i;
    struct termios newtio;
    char port[16];

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

    if (!appInit()) goto fail;

    while (1) {
        pif_Loop();

        pifTask_Loop();
    }

fail:
	appExit();

    stop_timer();

	return pif_enError;
}
