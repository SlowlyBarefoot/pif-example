#include "ex_servo.h"
#include "app_port.h"

#include <MsTimer2.h>
#include <Servo.h>


#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200


// The Servo library makes the pulses with Timer5 for up to 12 servos on the Mega, so it does not meet Timer2 of
// MsTimer2. Any digital pin will do.
static const uint8_t c_servo_pin[SERVO_COUNT] = { 9, 10, 11 };

static Servo s_servo[SERVO_COUNT];


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int rx_data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		rx_data = Serial.read();
		if (rx_data < 0) break;
		p_data[i] = rx_data;
	}
	return i;
}

static void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	digitalWrite(PIN_LED_L, state & 1);
}

static void actServoControl(uint8_t index, uint16_t pulse_us)
{
	Servo *p_servo = &s_servo[index];

	if (!pulse_us) {
		p_servo->detach();
		return;
	}

	if (!p_servo->attached()) {
		// Written before the attach as well, so that the first frame carries this pulse rather than the 1500 us the
		// library starts with. Until the attach it is held within the library's default range, hence the second.
		p_servo->writeMicroseconds(pulse_us);
		p_servo->attach(c_servo_pin[index], SERVO_PULSE_MIN, SERVO_PULSE_MAX);
	}
	p_servo->writeMicroseconds(pulse_us);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, NULL)) return FALSE;	// 200ms
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    if (!pifLog_InitHeap(1024)) return FALSE;													// 1024bytes
	if (!pifLog_AttachUart(&s_uart_log, 128)) return FALSE;										// 128bytes

    if (!pifServo_Init(&g_servo, PIF_ID_AUTO, SERVO_COUNT, SERVO_PERIOD, actServoControl)) return FALSE;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;
    return TRUE;
}
