// Do not remove the include below
#include "exProtocolSerialLoopS.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31


static uint8_t s_ucPinSwitch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

uint16_t actPushSwitchAcquire(PifSensor* p_owner)
{
	return !digitalRead(s_ucPinSwitch[p_owner->_id - PIF_ID_SWITCH]);
}

uint16_t actSerial1SendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial1.write((char *)pucBuffer, usSize);
}

BOOL actSerial1ReceiveData(PifUart *p_uart, uint8_t *pucData)
{
	int rxData;

	(void)p_uart;

	rxData = Serial1.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

uint16_t actSerial2SendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial2.write((char *)pucBuffer, usSize);
}

BOOL actSerial2ReceiveData(PifUart *p_uart, uint8_t *pucData)
{
	int rxData;

	(void)p_uart;

	rxData = Serial2.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
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
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	Serial.begin(115200);
	Serial1.begin(115200);
	Serial2.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
