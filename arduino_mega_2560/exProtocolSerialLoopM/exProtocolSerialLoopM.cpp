// Do not remove the include below
#include <MsTimer2.h>

#include "exProtocolSerialLoopM.h"
#include "appMain.h"
#ifdef USE_USART
#include "../usart.h"
#endif


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31


static uint8_t s_ucPinSwitch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


#ifdef USE_SERIAL

uint16_t actLogSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

#endif

#ifdef USE_USART

BOOL actLogStartTransfer()
{
	return USART_StartTransfer(0);
}

ISR(USART0_UDRE_vect)
{
	USART_Send(0, &g_comm_log);
}

#endif

void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

uint16_t actPushSwitchAcquire(PifId usPifId)
{
	return !digitalRead(s_ucPinSwitch[usPifId - PIF_ID_SWITCH]);
}

#ifdef USE_SERIAL

uint16_t actSerial1SendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial1.write((char *)pucBuffer, usSize);
}

BOOL actSerial1ReceiveData(PifComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = Serial1.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

uint16_t actSerial2SendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial2.write((char *)pucBuffer, usSize);
}

BOOL actSerial2ReceiveData(PifComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = Serial2.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

#endif

#ifdef USE_USART

BOOL actUart1StartTransfer()
{
	return USART_StartTransfer(1);
}

ISR(USART1_UDRE_vect)
{
	USART_Send(1, &g_serial1);
}

ISR(USART1_RX_vect)
{
	USART_Receive(1, &g_serial1);
}

BOOL actUart2StartTransfer()
{
	return USART_StartTransfer(2);
}

ISR(USART2_UDRE_vect)
{
	USART_Send(2, &g_serial2);
}

ISR(USART2_RX_vect)
{
	USART_Receive(2, &g_serial2);
}

#endif

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

#ifdef USE_SERIAL
	Serial.begin(115200);
	Serial1.begin(115200);
	Serial2.begin(115200);
#endif
#ifdef USE_USART
	USART_Init(0, 115200, DATA_BIT_DEFAULT | PARITY_DEFAULT | STOP_BIT_DEFAULT, FALSE);
	USART_Init(1, 115200, DATA_BIT_DEFAULT | PARITY_DEFAULT | STOP_BIT_DEFAULT, TRUE);
	USART_Init(2, 115200, DATA_BIT_DEFAULT | PARITY_DEFAULT | STOP_BIT_DEFAULT, TRUE);

	//Enable Global Interrupts
	sei();
#endif

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
