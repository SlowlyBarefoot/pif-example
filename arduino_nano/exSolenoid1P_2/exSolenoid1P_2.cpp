// Do not remove the include below
#include <MsTimer2.h>

#include "exSolenoid1P_2.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_RELAY_1CH			5

#define TASK_SIZE				3
#define TIMER_1MS_SIZE			3


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actSolenoidOrder(SWITCH swOrder, PifSolenoidDir enDir)
{
	(void)enDir;

	digitalWrite(PIN_RELAY_1CH, swOrder);

	pifLog_Printf(LT_INFO, "_SolenoidOrder(%d)", swOrder);
}

static uint16_t taskLedToggle(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
	return 0;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	pinMode(PIN_RELAY_1CH, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

    pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifSolenoid_Init(&g_solenoid, PIF_ID_AUTO, &g_timer_1ms, ST_1POINT, 0, actSolenoidOrder)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
