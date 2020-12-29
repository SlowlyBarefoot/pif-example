/**
 * Pulse 1개를 생성후 이를 1ms Timer에 연결한다.
 * 이 Pulse로 200ms후 적색 LED를 토글시키고 500ms후 노란색 LED를 토클시키고
 * 1000ms후 녹색 LED를 토글시키는 이 과정을 반복한다.
 * 여기서 Pulse의 PT_enOnce 형식을 알 수 있다.
 *
 * Create one pulse and connect it to the 1 ms timer.
 * Repeat the process of toggling the red LED after 200 ms with this pulse,
 * toggling the yellow LED after 500 ms, and toggling the green LED after 1000 ms.
 * Here you can see the PT_enOnce format of Pulse.
 */

// Do not remove the include below
#include <MsTimer2.h>

#include "exTimer3.h"

#include "pifPulse.h"


#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_LED_GREEN			4

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              3


static PIF_stPulse *g_pstTimer1ms = NULL;
static PIF_stPulseItem *pstTimer1msRed = NULL;
static PIF_stPulseItem *pstTimer1msYellow = NULL;
static PIF_stPulseItem *pstTimer1msGreen = NULL;


void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(g_pstTimer1ms);
}

static void led_red_toggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_RED, sw);
	sw ^= 1;

	pifPulse_StartItem(pstTimer1msYellow, 500);	// 500 * 1ms = 0.5sec
}

static void led_yellow_toggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_YELLOW, sw);
	sw ^= 1;

	pifPulse_StartItem(pstTimer1msGreen, 1000);	// 1000 * 1ms = 1sec
}

static void led_green_toggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_GREEN, sw);
	sw ^= 1;

	pifPulse_StartItem(pstTimer1msRed, 200);	// 200 * 1ms = 0.2sec
}


//The setup function is called once at startup of the sketch
void setup()
{
	PIF_unDeviceCode unDeviceCode = 1;

	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	pif_Init();

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(unDeviceCode++, PULSE_ITEM_COUNT);
    if (!g_pstTimer1ms) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%

    pstTimer1msRed = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
    if (!pstTimer1msRed) return;
    pifPulse_AttachEvtFinish(pstTimer1msRed, led_red_toggle, NULL);
    pifPulse_StartItem(pstTimer1msRed, 200);						// 200ms

    pstTimer1msYellow = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
    if (!pstTimer1msYellow) return;
    pifPulse_AttachEvtFinish(pstTimer1msYellow, led_yellow_toggle, NULL);

    pstTimer1msGreen = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
    if (!pstTimer1msGreen) return;
    pifPulse_AttachEvtFinish(pstTimer1msGreen, led_green_toggle, NULL);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
