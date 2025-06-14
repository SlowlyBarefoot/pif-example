// Do not remove the include below
#include "exGpio1.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25

#define PIN_PUSH_SWITCH			29
#define PIN_TILT_SWITCH			31

#define TASK_SIZE				2

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actGpioLedL(PifId usPifId, uint8_t ucState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, ucState);
}

static void actGpioLedRG(PifId usPifId, uint8_t ucState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_RED, ucState & 1);
	digitalWrite(PIN_LED_YELLOW, (ucState >> 1) & 1);
}

static uint8_t actGpioSwitch(PifId usPifId)
{
	(void)usPifId;

	return digitalRead(PIN_PUSH_SWITCH) + (digitalRead(PIN_TILT_SWITCH) << 1);
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);
	pinMode(PIN_TILT_SWITCH, INPUT_PULLUP);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return;							// 256bytes

    if (!pifGpio_Init(&g_gpio_l, PIF_ID_AUTO, 1)) return;
    pifGpio_AttachActOut(&g_gpio_l, actGpioLedL);

    if (!pifGpio_Init(&g_gpio_rg, PIF_ID_AUTO, 2)) return;
    pifGpio_AttachActOut(&g_gpio_rg, actGpioLedRG);

    if (!pifGpio_Init(&g_gpio_switch, PIF_ID_AUTO, 1)) return;
    pifGpio_AttachActIn(&g_gpio_switch, actGpioSwitch);

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***              exGpio1             ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
