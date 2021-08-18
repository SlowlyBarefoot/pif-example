#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "main.h"
#include "appMain.h"
#include "timer.h"


static int s_fd;


uint16_t actLogSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return fwrite((char *)pucBuffer, 1, usSize, stdout);
}

uint16_t actSerialSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
    return write(s_fd, pucBuffer, usSize);
}

BOOL actSerialReceiveData(PIF_stComm *pstComm, uint8_t *pucData)
{
	uint8_t data;

	if (read(s_fd, &data, 1)) {
		*pucData = data;
		if (data == 'C') printf("C");
		return TRUE;
	}
	return FALSE;
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
