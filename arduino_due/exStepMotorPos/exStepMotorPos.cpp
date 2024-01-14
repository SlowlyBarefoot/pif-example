// Do not remove the include below
#include <DueTimer.h>

#include "exStepMotorPos.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_STEP_MOTOR_1		30
#define PIN_STEP_MOTOR_2		32
#define PIN_STEP_MOTOR_3		34
#define PIN_STEP_MOTOR_4		36

#define PIN_PHOTO_INTERRUPT_1	35
#define PIN_PHOTO_INTERRUPT_2	37
#define PIN_PHOTO_INTERRUPT_3	39

#define TASK_SIZE				10
#define TIMER_1MS_SIZE			1
#define TIMER_200US_SIZE		3

#define UART_LOG_BAUDRATE		115200

#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


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

static void actSetStep(uint16_t usPhase)
{
	digitalWrite(PIN_STEP_MOTOR_1, usPhase & 1);
	digitalWrite(PIN_STEP_MOTOR_2, (usPhase >> 1) & 1);
	digitalWrite(PIN_STEP_MOTOR_3, (usPhase >> 2) & 1);
	digitalWrite(PIN_STEP_MOTOR_4, (usPhase >> 3) & 1);
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

static void _sigTimer200us()
{
	pifTimerManager_sigTick(&g_timer_200us);
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_STEP_MOTOR_1, OUTPUT);
	pinMode(PIN_STEP_MOTOR_2, OUTPUT);
	pinMode(PIN_STEP_MOTOR_3, OUTPUT);
	pinMode(PIN_STEP_MOTOR_4, OUTPUT);
	pinMode(PIN_PHOTO_INTERRUPT_1, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_2, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_3, INPUT_PULLUP);

	Timer3.attachInterrupt(_sigTimer200us).start(200);

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;			// 1000us

    if (!pifTimerManager_Init(&g_timer_200us, PIF_ID_AUTO, 200, TIMER_200US_SIZE)) return;		// 200us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;						// 1ms
	s_uart_log.act_receive_data = actLogReceiveData;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    if (!pifStepMotorPos_Init(&g_motor, PIF_ID_AUTO, &g_timer_200us,
    		STEP_MOTOR_RESOLUTION, SMO_2P_4W_1S, 100)) return;									// 100ms
    pifStepMotor_SetReductionGearRatio(&g_motor.parent, STEP_MOTOR_REDUCTION_GEAR_RATIO);
    g_motor.parent.act_set_step = actSetStep;

    for (int i = 0; i < SWITCH_COUNT; i++) {
		if (!pifSensorSwitch_Init(&g_switch[i], PIF_ID_SWITCH + i, 0, actPhotoInterruptAcquire)) return;
    }

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***          exStepMotorPos          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
    pifLog_Printf(LT_INFO, "Task=%d/%d Timer1ms=%d/%d Timer200us=%d/%d\n", pifTaskManager_Count(), TASK_SIZE,
    		pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE, pifTimerManager_Count(&g_timer_200us), TIMER_200US_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
