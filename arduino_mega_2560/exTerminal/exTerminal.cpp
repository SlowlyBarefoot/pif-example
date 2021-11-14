// Do not remove the include below
#include <MsTimer2.h>

#include "exTerminal.h"
#include "appMain.h"
#ifdef USE_USART
#include "../usart.h"
#endif

#include "pif_log.h"


#define PIN_LED_L				13


#ifdef USE_SERIAL

uint16_t actLogSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

BOOL actLogReceiveData(PifComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = Serial.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

#endif

#ifdef USE_USART

BOOL actLogStartTransfer()
{
	return USART_StartTransfer(0);
}

ISR(USART0_UDRE_vect)
{
	USART_Send(0, g_pstCommLog);
}

ISR(USART0_RX_vect)
{
	USART_Receive(0, g_pstCommLog);
}

#endif

void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
	pifLog_Printf(LT_INFO, "LED State=%u", unState);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(g_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

#ifdef USE_SERIAL
	Serial.begin(115200);
#endif
#ifdef USE_USART
	USART_Init(0, 115200, DATA_BIT_DEFAULT | PARITY_DEFAULT | STOP_BIT_DEFAULT, TRUE);

	// Enable Global Interrupts
	sei();
#endif

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
