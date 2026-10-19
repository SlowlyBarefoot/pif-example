#include "exSolenoid2P_B.h"
#include "app_port.h"


#define PIN_LED_L				13
#define PIN_L298N_ENB_PWM		2
#define PIN_L298N_IN3			44
#define PIN_L298N_IN4			46

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actSolenoidOrder(uint8_t index, SWITCH swOrder, PifSolenoidDir enDir)
{
	(void)index;

	if (swOrder) {
		switch (enDir) {
		case SD_LEFT:
			digitalWrite(PIN_L298N_IN3, HIGH);
			digitalWrite(PIN_L298N_IN4, LOW);
			break;

		case SD_RIGHT:
			digitalWrite(PIN_L298N_IN3, LOW);
			digitalWrite(PIN_L298N_IN4, HIGH);
			break;

		default:
			break;
		}
		analogWrite(PIN_L298N_ENB_PWM, 255);
	}
	else {
		analogWrite(PIN_L298N_ENB_PWM, 0);
	}
	pifLog_Printf(LT_INFO, "_SolenoidOrder(%d, %d)", enDir, swOrder);
}

static uint32_t taskLedToggle(PifTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
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

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_L298N_ENB_PWM, OUTPUT);
	pinMode(PIN_L298N_IN3, OUTPUT);
	pinMode(PIN_L298N_IN4, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;												// 256bytes

    if (!pifSolenoid_Init(&g_solenoid, PIF_ID_AUTO, 1,
    		ST_2POINT, 30, actSolenoidOrder)) goto fail;												// 30ms

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) goto fail;	// 500ms
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
