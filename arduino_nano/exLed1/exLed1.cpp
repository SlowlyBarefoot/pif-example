// Do not remove the include below
#include <MsTimer2.h>

#include "exLed1.h"
#include "appMain.h"
#ifdef USE_USART
#include "../usart.h"
#endif

#include "pif_log.h"


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_LED_GREEN			4


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
	return USART_StartTransfer();
}

ISR(USART_UDRE_vect)
{
	USART_Send(&g_comm_log);
}

#endif

void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

void actLedRGBState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_RED, unState & 1);
	digitalWrite(PIN_LED_YELLOW, (unState >> 1) & 1);
	digitalWrite(PIN_LED_GREEN, (unState >> 2) & 1);
	pifLog_Printf(LT_INFO, "RGB:%u S:%xh", __LINE__, unState);
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
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

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
