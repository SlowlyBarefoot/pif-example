/**
 * Pulse 1개를 생성후 이를 1ms Timer에 연결한다.
 * 이 Pulse로 200ms마다 적색 LED를 10번 깜박이고 500ms마다 노란색 LED를 10회 깜박이고
 * 1000ms마다 녹색 LED를 10회 깜박이는 이 과정을 반복한다.
 * 여기서 Pulse의 시작과 종료 함수 사용법과 이벤트 함수의 매개변수 사용법을 알 수 있다.
 *
 * Create one pulse and connect it to the 1 ms timer.
 * Repeat this process of flashing red LED 10 times every 200 ms, flashing yellow LED 10 times every 500 ms and
 * green LED 10 times every 1000 ms with this pulse.
 * Here you can see how to use the start and end functions of Pulse and how to use parameters of the event function.
 */

// Do not remove the include below
#include <MsTimer2.h>

#include "exTimer4.h"

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
	static BOOL sw = HIGH;
	static int count = 0;

	(void)pvIssuer;

	digitalWrite(PIN_LED_RED, sw);
	sw ^= 1;

	count++;
	if (count >= 10) {
		count = 0;
		pifPulse_StopItem((PIF_stPulseItem *)pvIssuer);
		pifPulse_StartItem(pstTimer1msYellow, 500);	// 500 * 1ms = 0.5sec
	}
}

static void led_yellow_toggle(void *pvIssuer)
{
	static BOOL sw = HIGH;
	static int count = 0;

	(void)pvIssuer;

	digitalWrite(PIN_LED_YELLOW, sw);
	sw ^= 1;

	count++;
	if (count >= 10) {
		count = 0;
		pifPulse_StopItem((PIF_stPulseItem *)pvIssuer);
		pifPulse_StartItem(pstTimer1msGreen, 1000);	// 1000 * 1ms = 1sec
	}
}

static void led_green_toggle(void *pvIssuer)
{
	static BOOL sw = HIGH;
	static int count = 0;

	(void)pvIssuer;

	digitalWrite(PIN_LED_GREEN, sw);
	sw ^= 1;

	count++;
	if (count >= 10) {
		count = 0;
		pifPulse_StopItem((PIF_stPulseItem *)pvIssuer);
		pifPulse_StartItem(pstTimer1msRed, 200);	// 200 * 1ms = 0.2sec
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	pif_Init();

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!g_pstTimer1ms) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) return;

    pstTimer1msRed = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1msRed) return;
    pifPulse_AttachEvtFinish(pstTimer1msRed, led_red_toggle, pstTimer1msRed);
    pifPulse_StartItem(pstTimer1msRed, 200);	// 200 * 1ms = 0.2sec

    pstTimer1msYellow = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1msYellow) return;
    pifPulse_AttachEvtFinish(pstTimer1msYellow, led_yellow_toggle, pstTimer1msYellow);

    pstTimer1msGreen = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimer1msGreen) return;
    pifPulse_AttachEvtFinish(pstTimer1msGreen, led_green_toggle, pstTimer1msGreen);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}