// Do not remove the include below
#include "pifTask.h"

#include "exTask2.h"


#define PIN_DUE_LED				13

#define TASK_COUNT              5


static int s_nTaskNo = 0;
static int s_nTaskCount = 0;
static PIF_stTask *s_pstTask[TASK_COUNT] = { NULL, NULL, NULL, NULL, NULL };

static void led_toggle(PIF_stTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;

    if (!nCount) {
		digitalWrite(PIN_DUE_LED, sw);
		sw ^= 1;

		s_nTaskCount++;
		if (s_nTaskCount >= 10) {
			s_nTaskCount = 0;
		    pifTask_Pause(s_pstTask[s_nTaskNo]);
		    s_nTaskNo++;
		    if (s_nTaskNo >= TASK_COUNT) s_nTaskNo = 0;
		    pifTask_Restart(s_pstTask[s_nTaskNo]);
		}

		nCount = 9999;
    }
    else nCount--;
}

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here
	pinMode(PIN_DUE_LED, OUTPUT);

	pif_Init();

    if (!pifTask_Init(TASK_COUNT)) return;

    s_pstTask[0] = pifTask_Add(20, led_toggle, NULL);
    if (!s_pstTask[0]) return;

    s_pstTask[1] = pifTask_Add(40, led_toggle, NULL);
    if (!s_pstTask[1]) return;
    pifTask_Pause(s_pstTask[1]);

    s_pstTask[2] = pifTask_Add(60, led_toggle, NULL);
    if (!s_pstTask[2]) return;
    pifTask_Pause(s_pstTask[2]);

    s_pstTask[3] = pifTask_Add(80, led_toggle, NULL);
    if (!s_pstTask[3]) return;
    pifTask_Pause(s_pstTask[3]);

    s_pstTask[4] = pifTask_Add(100, led_toggle, NULL);
    if (!s_pstTask[4]) return;
    pifTask_Pause(s_pstTask[4]);
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
    pif_Loop();

    pifTask_Loop();
}
