// Do not remove the include below
#include <MsTimer2.h>

#include "exXmodemSerialRx.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_PUSH_SWITCH			2

#define TASK_SIZE				4
#define TIMER_1MS_SIZE			3


static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

static uint16_t actPushSwitchAcquire(PifSensor* p_owner)
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

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

    pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

	if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, 0, actPushSwitchAcquire)) return;

	if (!pifUart_Init(&g_serial, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_serial, TM_PERIOD_MS, 1, "UartSerial")) return;				// 1ms
    g_serial.act_receive_data = actXmodemReceiveData;
    g_serial.act_send_data = actXmodemSendData;

	if (!appSetup()) return;
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
