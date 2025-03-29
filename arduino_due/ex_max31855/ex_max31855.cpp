// Do not remove the include below
#include "ex_max31855.h"
#include "app_main.h"

#include <SPI.h>


#define PIN_LED_L				13

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			2

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	digitalWrite(PIN_LED_L, state & 1);
}

static void actTransfer(PifSpiDevice *p_owner, uint8_t* p_write, uint8_t* p_read, size_t size)
{
	uint16_t i;

	(void)p_owner;

	digitalWrite(SS, LOW);
	if (p_write) {
		if (p_read) {
			for (i = 0; i < size; i++) {
				p_read[i] = SPI.transfer(p_write[i]);
			}
		}
		else {
			for (i = 0; i < size; i++) {
				SPI.transfer(p_write[i]);
			}
		}
	}
	else {
		if (p_read) {
			for (i = 0; i < size; i++) {
				p_read[i] = SPI.transfer(0);
			}
		}
	}
	digitalWrite(SS, HIGH);
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
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	SPI.begin();
//	SPI.setClockDivider(SPI_CLOCK_DIV16);
	digitalWrite(SS, HIGH);

	Serial.begin(UART_LOG_BAUDRATE);

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD, 1000, NULL)) return;					// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifSpiPort_Init(&g_spi_port, PIF_ID_AUTO, 1)) return;
    g_spi_port.act_transfer = actTransfer;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***           ex_max31855            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
