// Do not remove the include below
#include "exFnd1S.h"
#include "appMain.h"


#define PIN_LED_L				13

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1


const uint8_t c_unPinFnd[] = {
		43,		// a
		45, 	// b
		39, 	// c
		37, 	// d
		35, 	// e
		33, 	// f
		31, 	// g
		41 		// dp
};


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit)
{
	(void)ucDigit;

	for (int j = 0; j < 8; j++) {
		digitalWrite(c_unPinFnd[j], (ucSegment >> j) & 1);
	}
}

static uint16_t taskLedToggle(PifTask *pstTask)
{
	static BOOL swLed = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
	return 0;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	for (int i = 0; i < 8; i++) {
		pinMode(c_unPinFnd[i], OUTPUT);
	}

	Serial.begin(115200); //Doesn't matter speed

    pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifFnd_Init(&g_fnd, PIF_ID_AUTO, &g_timer_1ms, 1, actFndDisplay)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
