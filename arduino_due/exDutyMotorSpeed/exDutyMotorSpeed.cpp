// Do not remove the include below
#include "exDutyMotorSpeed.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_L298N_ENB_PWM		2
#define PIN_L298N_IN1			30
#define PIN_L298N_IN2			32

#define PIN_PHOTO_INTERRUPT_1	35
#define PIN_PHOTO_INTERRUPT_2	37
#define PIN_PHOTO_INTERRUPT_3	39

#define TASK_SIZE				8
#define TIMER_1MS_SIZE			3


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size, uint8_t* p_rate)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	if (p_rate) *p_rate = 100 * Serial.available() / SERIAL_BUFFER_SIZE;
	return i;
}

static uint16_t actPhotoInterruptAcquire(PifSensor* p_owner)
{
	switch (p_owner->_id) {
	case PIF_ID_SWITCH:
		return digitalRead(PIN_PHOTO_INTERRUPT_1);

	case PIF_ID_SWITCH + 1:
		return digitalRead(PIN_PHOTO_INTERRUPT_2);

	case PIF_ID_SWITCH + 2:
		return digitalRead(PIN_PHOTO_INTERRUPT_3);
	}
	return OFF;
}

static void actSetDuty(uint16_t usDuty)
{
	analogWrite(PIN_L298N_ENB_PWM, usDuty);
	pifLog_Printf(LT_INFO, "SetDuty(%d)", usDuty);
}

static void actSetDirection(uint8_t ucDir)
{
	if (ucDir) {
		digitalWrite(PIN_L298N_IN1, LOW);
		digitalWrite(PIN_L298N_IN2, HIGH);
	}
	else {
		digitalWrite(PIN_L298N_IN1, HIGH);
		digitalWrite(PIN_L298N_IN2, LOW);
	}
	pifLog_Printf(LT_INFO, "SetDirection(%d)", ucDir);
}

static void actOperateBreak(uint8_t ucState)
{
	if (ucState) {
		analogWrite(PIN_L298N_ENB_PWM, 255);
		digitalWrite(PIN_L298N_IN1, LOW);
		digitalWrite(PIN_L298N_IN2, LOW);
	}
	else {
		analogWrite(PIN_L298N_ENB_PWM, 0);
	}
	pifLog_Printf(LT_INFO, "OperateBreak(%d)", ucState);
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
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
	pinMode(PIN_L298N_ENB_PWM, OUTPUT);
	pinMode(PIN_L298N_IN1, OUTPUT);
	pinMode(PIN_L298N_IN2, OUTPUT);
	pinMode(PIN_PHOTO_INTERRUPT_1, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_2, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_3, INPUT_PULLUP);

	Serial.begin(115200);

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;						// 1ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    if (!pifDutyMotorSpeed_Init(&g_motor, PIF_ID_AUTO, &g_timer_1ms, 255, 100)) return;			// 100ms
    g_motor.parent.act_set_duty = actSetDuty;
    g_motor.parent.act_set_direction = actSetDirection;
    g_motor.parent.act_operate_break = actOperateBreak;

    for (int i = 0; i < SWITCH_COUNT; i++) {
		if (!pifSensorSwitch_Init(&g_switch[i], PIF_ID_SWITCH + i, 0, actPhotoInterruptAcquire)) return;
    }

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
