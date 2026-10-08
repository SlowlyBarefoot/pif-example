// Do not remove the include below
#include <MsTimer2.h>

#include "ex_link_serial.h"
#include "app_main.h"


#define PIN_LED_L				13

#define TASK_SIZE				6
#define TIMER_1MS_SIZE			3

#define UART_SERIAL_BAUDRATE	115200


static void actLedLState(PifId pid_id, uint32_t state)
{
	(void)pid_id;

	digitalWrite(PIN_LED_L, state & 1);
}

static uint16_t actSerialSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static uint16_t actSerialReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_SERIAL_BAUDRATE);

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

	if (!pifUart_Init(&g_serial, PIF_ID_AUTO, UART_SERIAL_BAUDRATE)) return;
    if (!pifUart_AttachTask(&g_serial, TM_PERIOD, 1000, NULL)) return;						// 1ms
    g_serial.act_receive_data = actSerialReceiveData;
    g_serial.act_send_data = actSerialSendData;

	if (!appSetup()) return;
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
