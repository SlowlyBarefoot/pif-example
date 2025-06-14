// Do not remove the include below
#include "exDutyMotorPos.h"
#include "appMain.h"

#include "core/pif_pulse.h"


#define PIN_LED_L				13

#define PIN_L298N_ENB_PWM		2
#define PIN_L298N_IN1			30
#define PIN_L298N_IN2			32

#define PIN_ENCODER				34

#define PIN_PHOTO_INTERRUPT_1	35
#define PIN_PHOTO_INTERRUPT_2	37
#define PIN_PHOTO_INTERRUPT_3	39

#define TASK_SIZE				10
#define TIMER_1MS_SIZE			3

#define UART_LOG_BAUDRATE		115200


static PifPulse s_encoder;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
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

static void _isrEncoder()
{
	pifPulse_sigEdge(&s_encoder, digitalRead(PIN_ENCODER) ? PS_RISING_EDGE : PS_FALLING_EDGE, micros());
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

	pinMode(PIN_ENCODER, INPUT_PULLUP);
	attachInterrupt(PIN_ENCODER, _isrEncoder, CHANGE);

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;				// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, NULL)) return;			// 200ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return;												// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    if (!pifPulse_Init(&s_encoder, PIF_ID_AUTO)) return;

    if (!pifDutyMotorPos_Init(&g_motor, PIF_ID_AUTO, &g_timer_1ms, 255, 100, &s_encoder)) return;	// 100ms
    g_motor.parent.act_set_duty = actSetDuty;
    g_motor.parent.act_set_direction = actSetDirection;
    g_motor.parent.act_operate_break = actOperateBreak;

    for (int i = 0; i < SWITCH_COUNT; i++) {
		if (!pifSensorSwitch_Init(&g_switch[i], PIF_ID_SWITCH + i, 0, actPhotoInterruptAcquire)) return;
    }

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***          exDutyMotorPos          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
