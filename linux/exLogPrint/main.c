/**
 * 1초마다 log를 출력한다.
 *
 * Output log every second.
 */

#include "timer.h"

#include "pifLog.h"
#include "pifPulse.h"


#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              1


static volatile uint16_t s_unTimer = 30000;

PIF_stPulse *g_pstTimer = NULL;


static void log_print(char *cString)
{
	printf("%s", cString);
}

static void timer_handler()
{
	static int nTimer = 0;

    if (s_unTimer) s_unTimer--;

    pif_sigTimer1ms();

    pifPulse_sigTick(g_pstTimer);

    if (nTimer) nTimer--;
    else {
    	pifLog_Printf(LT_enInfo, "1sec Timer");
    	nTimer = 1000 - 1;
    }
}

int main(int argc, char **argv)
{
    if (start_timer(1, &timer_handler)) {     // 1ms
        printf("\nstart_timer error\n");
        return(1);
    }

    pif_Init();

    pifLog_Init();
    pifLog_AttachActPrint(log_print);

    if (!pifPulse_Init(PULSE_COUNT)) goto fail;
    g_pstTimer = pifPulse_Add(PULSE_ITEM_COUNT, 1000);    // 1000us Period * 20ea
    if (!g_pstTimer) goto fail;

    pifTask_Init(TASK_COUNT);

    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) goto fail;

    while (s_unTimer) {
        pif_Loop();

        pifTask_Loop();
    }

fail:
    pifTask_Exit();
    pifPulse_Exit();
    pifLog_Exit();

    stop_timer();

	return pif_enError;
}
