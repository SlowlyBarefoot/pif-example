#include "ex_max31855.h"
#include "app_port.h"

#include <MsTimer2.h>
#include <SPI.h>


#define PIN_LED_L				13

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

	digitalWrite(SS, FALSE);
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
	digitalWrite(SS, TRUE);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV8);
	digitalWrite(SS, TRUE);

	Serial.begin(UART_LOG_BAUDRATE);

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;										// 256bytes

    if (!pifSpiPort_Init(&g_spi_port, PIF_ID_AUTO, 1)) goto fail;
    g_spi_port.act_transfer = actTransfer;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) goto fail;
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
