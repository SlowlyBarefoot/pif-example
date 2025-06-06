// Do not remove the include below
#include "ex_tft_ili9341_lvgl.h"
#include "linker.h"

#include <SPI.h>


#define PIN_LED_L				13

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define PIN_LCD_CS				A3 // Chip Select goes to Analog 3
#define PIN_LCD_RS				A2 // Command/Data goes to Analog 2
#define PIN_LCD_RST				A4 // Can alternately just connect to Arduino's reset pin

#define PIN_LCD_BL				7

#define PIN_LCD_WR				A1 // LCD Write goes to Analog 1
#define PIN_LCD_RD				A0 // LCD Read goes to Analog 0

#define PIN_TS_CS				3
#define PIN_TS_PEN				4

#define TASK_SIZE				10
#define TIMER_1MS_SIZE			3

#define UART_LOG_BAUDRATE		115200


const uint8_t lcd_setup[] = {
	ILI9341_CMD_POWER_CTRL_B, 			 3, 0x00, 0xD9, 0x30,
	ILI9341_CMD_POWER_SEQ_CTRL, 		 4, 0x64, 0x03, 0x12, 0x81,
	ILI9341_CMD_DRIVER_TIM_CTRL_A1, 	 3, 0x85, 0x10, 0x7A,
	ILI9341_CMD_POWER_CTRL_A, 			 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
	ILI9341_CMD_PUMP_RATIO_CTRL, 		 1, 0x20,
	ILI9341_CMD_DRIVER_TIM_CTRL_B, 		 2, 0x00, 0x00,
	ILI9341_CMD_POWER_CTRL1, 			 1, 0x1B,
	ILI9341_CMD_POWER_CTRL2, 			 1, 0x12,
	ILI9341_CMD_VCOM_CTRL1, 			 2, 0x26, 0x26,
	ILI9341_CMD_VCOM_CTRL2, 			 1, 0xB0,
	ILI9341_CMD_MEM_ACCESS_CTRL, 		 1, ILI9341_MADCTL_BGR,
	ILI9341_CMD_PIXEL_FORMAT_SET, 		 1, (ILI9341_PF_16BIT << 4) | ILI9341_PF_16BIT,
	ILI9341_CMD_FRAME_RATE_CTRL_NORMAL,  2, 0x00, 0x1A,
	ILI9341_CMD_DISP_FUNC_CTRL, 	 	 2, 0x0A, 0xA2,
	ILI9341_CMD_ENABLE_3G, 			 	 1, 0x00,
	ILI9341_CMD_GAMMA_SET, 			 	 1, 0x01,
	ILI9341_CMD_POS_GAMMA_CORRECT, 		15, 0x1F, 0x24, 0x24, 0x0D, 0x12, 0x09, 0x52, 0xB7, 0x3F, 0x0C, 0x15, 0x06, 0x0E, 0x08, 0x00,
	ILI9341_CMD_NEG_GAMMA_CORRECT, 		15, 0x00, 0x1B, 0x1B, 0x02, 0x0E, 0x06, 0x2E, 0x48, 0x3F, 0x03, 0x0A, 0x09, 0x31, 0x37, 0x1F,
	ILI9341_CMD_PAGE_ADDR_SET, 			 4, 0x00, 0x00, 0x01, 0x3F,
	ILI9341_CMD_COL_ADDR_SET, 			 4, 0x00, 0x00, 0x00, 0xEF,
	ILI9341_CMD_NOP
};
const uint8_t lcd_rotation[] = {
	ILI9341_MADCTL_BGR,
	ILI9341_MADCTL_MV | ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
	ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
	ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR
};

static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_L, state & 1);
}

static uint16_t actLogSendData(PifUart* p_uart, uint8_t* p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
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

static void actLcdReset()
{
	digitalWrite(PIN_LCD_RST, LOW);
	delay(100);
	digitalWrite(PIN_LCD_RST, HIGH);
	delay(50);
}

static void actLcdChipSelect(SWITCH sw)
{
	digitalWrite(PIN_LCD_CS, sw ? LOW : HIGH);
}

static uint32_t _LcdReadBus()
{
	uint32_t t, d;

	digitalWrite(PIN_LCD_RD, LOW);
	delayMicroseconds(1);
	t = PIOC->PIO_PDSR;
	d = ((t >> 2) & 0xFF) | ((t >> 4) & 0xFF00);
	digitalWrite(PIN_LCD_RD, HIGH);
	return d;
}

static void _LcdWritBus(uint32_t d)
{
	PIO_Set(PIOC,    (((d) & 0x00FF) << 2) |  (((d) & 0xFF00) << 4));
	PIO_Clear(PIOC, (((~d) & 0x00FF) << 2) | (((~d) & 0xFF00) << 4));
	digitalWrite(PIN_LCD_WR, LOW);
	digitalWrite(PIN_LCD_WR, HIGH);
}

static void actLcdReadCmd(PifTftLcdCmd cmd, uint32_t* p_data, uint32_t size)
{
	digitalWrite(PIN_LCD_RS, LOW);
	_LcdWritBus(cmd);

	if (size) {
		digitalWrite(PIN_LCD_RS, HIGH);
		pmc_enable_periph_clk( ID_PIOC );
		PIOC->PIO_PUDR |=  0x000FF3FC; PIOC->PIO_IFDR |=  0x000FF3FC; PIOC->PIO_ODR |=  0x000FF3FC; PIOC->PIO_PER |=  0x000FF3FC;
		delayMicroseconds(1);
		for (uint32_t i = 0; i < size; i++) {
			p_data[i] = _LcdReadBus();
		}
		PIOC->PIO_MDDR |=  0x000FF3FC; PIOC->PIO_OER |=  0x000FF3FC; PIOC->PIO_PER |=  0x000FF3FC;
	}
}

static void actLcdWriteCmd(PifTftLcdCmd cmd, uint32_t* p_data, uint32_t size)
{
	digitalWrite(PIN_LCD_RS, LOW);
	_LcdWritBus(cmd);

	if (size) {
		digitalWrite(PIN_LCD_RS, HIGH);
		for (uint32_t i = 0; i < size; i++) {
			_LcdWritBus(p_data[i]);
		}
	}
}

static void actLcdWriteData(uint32_t* p_data, uint32_t size)
{
	digitalWrite(PIN_LCD_RS, HIGH);
	for (uint32_t i = 0; i < size; i++) {
		_LcdWritBus(p_data[i]);
	}
}

static void actLcdWriteRepeat(uint32_t* p_data, uint8_t size, uint32_t len)
{
	uint8_t p = 0;

	digitalWrite(PIN_LCD_RS, HIGH);
	for (uint32_t i = 0; i < size * len; i++) {
		_LcdWritBus(p_data[p]);
		p++;
		if (p >= size) p = 0;
	}
}

static BOOL actPen()
{
	return !digitalRead(PIN_TS_PEN);
}

static void actTransfer(PifSpiDevice *p_owner, uint8_t* p_write, uint8_t* p_read, size_t size)
{
	uint16_t i;

	(void)p_owner;

	digitalWrite(PIN_TS_CS, LOW);
	digitalWrite(SS, LOW);
	SPI.transfer(p_write[0]);
	for (i = 0; i < size; i++) {
		p_read[i] = SPI.transfer(0);
	}
	digitalWrite(SS, HIGH);
	digitalWrite(PIN_TS_CS, HIGH);
}

static void actLcdBackLight(uint8_t level)
{
	analogWrite(PIN_LCD_BL, (uint16_t)level * 255 / 100);
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
	int line;

	pinMode(PIN_LED_L, OUTPUT);

	pinMode(PIN_LCD_RS, OUTPUT);
	pinMode(PIN_LCD_CS, OUTPUT);
	pinMode(PIN_LCD_RST, OUTPUT);

	pinMode(PIN_LCD_RD, OUTPUT);
	pinMode(PIN_LCD_WR, OUTPUT);
	for (int i = 0; i < 8; i++) {
		pinMode(34 + i, OUTPUT);
		pinMode(44 + i, OUTPUT);
	}
	pinMode(PIN_TS_CS, OUTPUT);
	pinMode(PIN_TS_PEN, INPUT_PULLUP);
	pinMode(PIN_LCD_BL, OUTPUT);

	analogWrite(PIN_LCD_BL, 0x7F);

	SPI.begin();
//	SPI.setClockDivider(SPI_CLOCK_DIV16);
	digitalWrite(SS, HIGH);

	digitalWrite(PIN_LCD_RD, HIGH);
	digitalWrite(PIN_LCD_WR, HIGH);
	digitalWrite(PIN_LCD_RS, HIGH);
	digitalWrite(PIN_LCD_CS, HIGH);
	digitalWrite(PIN_LCD_RST, HIGH);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) { line = __LINE__; goto fail; }

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }				// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) { line = __LINE__; goto fail; }
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) { line = __LINE__; goto fail; }	// 200ms
    s_uart_log.act_receive_data = actLogReceiveData;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) { line = __LINE__; goto fail; }												// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) { line = __LINE__; goto fail; }					// 2EA

    if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_16BIT_I)) { line = __LINE__; goto fail; }
    g_ili9341.parent.act_backlight = actLcdBackLight;

    if (!pifSpiPort_Init(&g_spi_port, PIF_ID_AUTO, 1)) { line = __LINE__; goto fail; }
    g_spi_port.act_transfer = actTransfer;

    if (!pifTsc2046_Init(&g_tsc2046, PIF_ID_AUTO, &g_ili9341.parent, 3761, 229, 3899, 420, &g_spi_port, NULL, actPen)) { line = __LINE__; goto fail; }
    if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, actLcdReadCmd, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
    if (!pifIli9341_Setup(&g_ili9341, lcd_setup, lcd_rotation)) { line = __LINE__; goto fail; }

	if (!appSetup()) { line = __LINE__; goto fail; }

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***       ex_tft_ili9341_lvgl        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return;

fail:
	pifLog_Printf(LT_INFO, "Initial failed. %d\n", line);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
