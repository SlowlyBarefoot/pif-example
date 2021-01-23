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
#include "exTimer4.h"

#include "pifPulse.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              3


static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stPulseItem *s_pstTimer1msRed = NULL;
static PIF_stPulseItem *s_pstTimer1msYellow = NULL;
static PIF_stPulseItem *s_pstTimer1msGreen = NULL;


extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(s_pstTimer1ms);
	}
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
		pifPulse_StartItem(s_pstTimer1msYellow, 500);	// 500 * 1ms = 0.5sec
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
		pifPulse_StartItem(s_pstTimer1msGreen, 1000);	// 1000 * 1ms = 1sec
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
		pifPulse_StartItem(s_pstTimer1msRed, 200);	// 200 * 1ms = 0.2sec
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	pif_Init();

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%

    s_pstTimer1msRed = pifPulse_AddItem(s_pstTimer1ms, PT_enRepeat);
    if (!s_pstTimer1msRed) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msRed, led_red_toggle, s_pstTimer1msRed);
    pifPulse_StartItem(s_pstTimer1msRed, 200);						// 200ms

    s_pstTimer1msYellow = pifPulse_AddItem(s_pstTimer1ms, PT_enRepeat);
    if (!s_pstTimer1msYellow) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msYellow, led_yellow_toggle, s_pstTimer1msYellow);

    s_pstTimer1msGreen = pifPulse_AddItem(s_pstTimer1ms, PT_enRepeat);
    if (!s_pstTimer1msGreen) return;
    pifPulse_AttachEvtFinish(s_pstTimer1msGreen, led_green_toggle, s_pstTimer1msGreen);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
