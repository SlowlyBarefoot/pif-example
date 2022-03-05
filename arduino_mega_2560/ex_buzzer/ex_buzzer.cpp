// Do not remove the include below
#include <MsTimer2.h>

#include "ex_buzzer.h"
#include "app_main.h"
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

#endif

#ifdef USE_USART

BOOL actLogStartTransfer(PifComm* p_comm)
{
	(void)p_comm;

	return USART_StartTransfer();
}

ISR(USART_UDRE_vect)
{
	USART_Send(&g_comm_log);
}

#endif

void actBuzzerAction(PifId id, BOOL action)
{
	(void)id;

	digitalWrite(PIN_LED_L, action);
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

#ifdef USE_SERIAL
	Serial.begin(115200); //Doesn't matter speed
#endif
#ifdef USE_USART
	USART_Init(115200, DATA_BIT_DEFAULT | PARITY_DEFAULT | STOP_BIT_DEFAULT, FALSE);

	// Enable Global Interrupts
	sei();
#endif

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
