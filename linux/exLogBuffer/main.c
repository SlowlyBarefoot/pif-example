/**
 * 1초마다 log를 log buffer에 저장하다가 프로그램 종료시 모두 출력한다.
 *
 * Save log to log buffer every second and print them out at the end of the program.
 */

#include "main.h"
#include "appMain.h"
#include "timer.h"

#include "pifLog.h"


static volatile uint16_t s_unTimer = 10000;


static void _TimerHandler()
{
	static int nTimer = 0;

    if (s_unTimer) s_unTimer--;

    pif_sigTimer1ms();

    if (nTimer) nTimer--;
    else {
    	pifLog_Printf(LT_enInfo, "1sec Timer");
    	nTimer = 1000 - 1;
    }
}

void actLogPrint(char *cString)
{
	printf("%s", cString);
}

int main(int argc, char **argv)
{
    if (start_timer(1, &_TimerHandler)) {     // 1ms
        printf("\nstart_timer error\n");
        return(1);
    }

    if (!appInit()) goto fail;

    while (s_unTimer) {
        pif_Loop();
    }

    pifLog_PrintInBuffer();

fail:
	appExit();

    stop_timer();

	return pif_enError;
}
