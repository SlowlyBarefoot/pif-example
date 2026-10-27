#include "exXmodemSerialRx.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH			2

#define UART_SERIAL_BAUDRATE	115200


static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

static int32_t actPushSwitchAcquire(PifSensor* p_owner)
{
	(void)p_owner;

	return !digitalRead(PIN_PUSH_SWITCH);
}

static uint16_t actXmodemSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actXmodemReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
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

BOOL portSetup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_SERIAL_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;

	if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, 0, actPushSwitchAcquire)) return FALSE;

	if (!pifUart_Init(&g_serial, PIF_ID_AUTO, UART_SERIAL_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&g_serial, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
    if (!pifUart_AttachRxTask(&g_serial, PIF_ID_AUTO, TM_PERIOD, 1000, NULL)) return FALSE;		// 1ms
    g_serial.act_receive_data = actXmodemReceiveData;
    g_serial.act_send_data = actXmodemSendData;
	return TRUE;
}
