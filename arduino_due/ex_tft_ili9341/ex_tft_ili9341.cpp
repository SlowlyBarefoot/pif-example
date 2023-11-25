// Do not remove the include below
#include "ex_tft_ili9341.h"
#include "linker.h"

#if LCD_TYPE == LCD_3_2_INCH
	#include <SPI.h>
#endif


#define PIN_LED_L				13

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define PIN_LCD_CS				A3 // Chip Select goes to Analog 3
#define PIN_LCD_RS				A2 // Command/Data goes to Analog 2
#define PIN_LCD_WR				A1 // LCD Write goes to Analog 1
#define PIN_LCD_RD				A0 // LCD Read goes to Analog 0
#define PIN_LCD_RST				A4 // Can alternately just connect to Arduino's reset pin

#if LCD_TYPE == LCD_2_4_INCH

#define PIN_TS_YP 				A1  // must be an analog pin, use "An" notation!
#define PIN_TS_XP 				A2  // must be an analog pin, use "An" notation!
#define PIN_TS_YM 				7   // can be a digital pin
#define PIN_TS_XM 				6   // can be a digital pin

#elif LCD_TYPE == LCD_3_2_INCH

#define PIN_LCD_BL				7

#define PIN_TS_CS				3
#define PIN_TS_PEN				4

#endif

#define TASK_SIZE				10
#define TIMER_1MS_SIZE			3


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

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size, uint8_t* p_rate)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	if (p_rate) *p_rate = 100 * Serial.available() / SERIAL_BUFFER_SIZE;
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
#if LCD_TYPE == LCD_2_4_INCH
	t = PIOC->PIO_PDSR;
	d = ((t >> (22 - 0)) & 1) | ((t >> (21 - 1)) & 2) | ((t >> (28 - 3)) & 8) | ((t >> (26 - 4)) & 0x10) | ((t >> (25 - 5)) & 0x20) | ((t >> (24 - 6)) & 0x40) | ((t >> (23 - 7)) & 0x80);
	t = PIOB->PIO_PDSR;
	d |= ((t >> (25 - 2)) & 4);
#elif LCD_TYPE == LCD_3_2_INCH
	t = PIOC->PIO_PDSR;
	d = ((t >> 2) & 0xFF) | ((t >> 4) & 0xFF00);
#endif
	digitalWrite(PIN_LCD_RD, HIGH);
	return d;
}

static void _LcdWritBus(uint32_t d)
{
#if LCD_TYPE == LCD_2_4_INCH
	PIO_Set(PIOC,    (((d) & 0x01) << (22 - 0)) |  (((d) & 0x02) << (21 - 1)) |  (((d) & 0x08) << (28 - 3)) |  (((d) & 0x10) << (26 - 4)) |  (((d) & 0x20) << (25 - 5)) |  (((d) & 0x40) << (24 - 6)) |  (((d) & 0x80) << (23 - 7)));
	PIO_Set(PIOB,    (((d) & 0x04) << (25 - 2)));
	PIO_Clear(PIOC, (((~d) & 0x01) << (22 - 0)) | (((~d) & 0x02) << (21 - 1)) | (((~d) & 0x08) << (28 - 3)) | (((~d) & 0x10) << (26 - 4)) | (((~d) & 0x20) << (25 - 5)) | (((~d) & 0x40) << (24 - 6)) | (((~d) & 0x80) << (23 - 7)));
	PIO_Clear(PIOB, (((~d) & 0x04) << (25 - 2)));
#elif LCD_TYPE == LCD_3_2_INCH
	PIO_Set(PIOC,    (((d) & 0x00FF) << 2) |  (((d) & 0xFF00) << 4));
	PIO_Clear(PIOC, (((~d) & 0x00FF) << 2) | (((~d) & 0xFF00) << 4));
#endif
	digitalWrite(PIN_LCD_WR, LOW);
	digitalWrite(PIN_LCD_WR, HIGH);
}

static void actLcdReadCmd(PifTftLcdCmd cmd, uint32_t* p_data, uint32_t size)
{
	digitalWrite(PIN_LCD_RS, LOW);
	_LcdWritBus(cmd);

	if (size) {
		digitalWrite(PIN_LCD_RS, HIGH);
#if LCD_TYPE == LCD_2_4_INCH
		pmc_enable_periph_clk( ID_PIOC ); pmc_enable_periph_clk( ID_PIOB );
		PIOC->PIO_PUDR |=  0x17E00000; PIOC->PIO_IFDR |=  0x17E00000; PIOC->PIO_ODR |=  0x17E00000; PIOC->PIO_PER |=  0x17E00000;
		PIOB->PIO_PUDR |=  0x02000000; PIOB->PIO_IFDR |=  0x02000000; PIOB->PIO_ODR |=  0x02000000; PIOB->PIO_PER |=  0x02000000;
#elif LCD_TYPE == LCD_3_2_INCH
		pmc_enable_periph_clk( ID_PIOC );
		PIOC->PIO_PUDR |=  0x000FF3FC; PIOC->PIO_IFDR |=  0x000FF3FC; PIOC->PIO_ODR |=  0x000FF3FC; PIOC->PIO_PER |=  0x000FF3FC;
#endif
		delayMicroseconds(1);
		for (uint32_t i = 0; i < size; i++) {
			p_data[i] = _LcdReadBus();
		}
#if LCD_TYPE == LCD_2_4_INCH
		PIOC->PIO_MDDR |=  0x17E00000; PIOC->PIO_OER |=  0x17E00000; PIOC->PIO_PER |=  0x17E00000;
		PIOB->PIO_MDDR |=  0x02000000; PIOB->PIO_OER |=  0x02000000; PIOB->PIO_PER |=  0x02000000;
#elif LCD_TYPE == LCD_3_2_INCH
		PIOC->PIO_MDDR |=  0x000FF3FC; PIOC->PIO_OER |=  0x000FF3FC; PIOC->PIO_PER |=  0x000FF3FC;
#endif
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

#if LCD_TYPE == LCD_2_4_INCH

static void actTouchPosition(PifTouchScreen* p_owner, int16_t* x, int16_t* y)
{
	(void)p_owner;

	digitalWrite(PIN_TS_YM, LOW);
	pinMode(PIN_TS_YP, INPUT);
	pinMode(PIN_TS_YM, INPUT);

	digitalWrite(PIN_TS_XP, HIGH);
	digitalWrite(PIN_TS_XM, LOW);

    *x = analogRead(PIN_TS_YP);

	pinMode(PIN_TS_YP, OUTPUT);
	pinMode(PIN_TS_YM, OUTPUT);

	digitalWrite(PIN_TS_XM, LOW);
	pinMode(PIN_TS_XP, INPUT);
	pinMode(PIN_TS_XM, INPUT);

	digitalWrite(PIN_TS_YP, HIGH);
    digitalWrite(PIN_TS_YM, LOW);

    *y = analogRead(PIN_TS_XP);

    pinMode(PIN_TS_XP, OUTPUT);
    pinMode(PIN_TS_XM, OUTPUT);
}

static BOOL actTouchPressure(PifTouchScreen* p_owner)
{
	int16_t z;

	(void)p_owner;

	// Set X+ to ground
	digitalWrite(PIN_TS_XM, LOW);

	// Set Y- to VCC
	digitalWrite(PIN_TS_YM, HIGH);

	// Hi-Z X- and Y+
    digitalWrite(PIN_TS_XP, LOW);
	digitalWrite(PIN_TS_YP, LOW);
    pinMode(PIN_TS_XP, INPUT);
	pinMode(PIN_TS_YP, INPUT);

	int z1 = analogRead(PIN_TS_XP);
	int z2 = analogRead(PIN_TS_YP);

    if (z1 < 10) z = 0;
    else z = 1023 - (z2 - z1);

    pinMode(PIN_TS_XP, OUTPUT);
    pinMode(PIN_TS_YP, OUTPUT);

    return z > 150;
}

#elif LCD_TYPE == LCD_3_2_INCH

static void actLcdBackLight(uint8_t level)
{
	analogWrite(PIN_LCD_BL, level * 255 / 100);
}

static BOOL actPen()
{
	return digitalRead(PIN_TS_PEN);
}

static void actTransfer(PifId id, uint8_t* p_write, uint8_t* p_read, uint16_t size)
{
	uint16_t i;

	(void)id;

	digitalWrite(PIN_TS_CS, LOW);
	digitalWrite(SS, LOW);
	SPI.transfer(p_write[0]);
	for (i = 0; i < size; i++) {
		p_read[i] = SPI.transfer(0);
	}
	digitalWrite(SS, HIGH);
	digitalWrite(PIN_TS_CS, HIGH);
}

#endif

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
#if LCD_TYPE == LCD_2_4_INCH
	const uint8_t lcd_setup[] = {
		ILI9341_CMD_POWER_CTRL_A, 			5, 0x39, 0x2C, 0x00, 0x34, 0x02,
		ILI9341_CMD_POWER_CTRL_B, 			3, 0x00, 0xC1, 0x30,
		ILI9341_CMD_DRIVER_TIM_CTRL_A1, 	3, 0x85, 0x00, 0x78,
		ILI9341_CMD_DRIVER_TIM_CTRL_B, 		2, 0x00, 0x00,
		ILI9341_CMD_POWER_SEQ_CTRL, 		4, 0x64, 0x03, 0x12, 0x81,
		ILI9341_CMD_PUMP_RATIO_CTRL, 		1, 0x20,
		ILI9341_CMD_POWER_CTRL1, 			1, 0x23,
		ILI9341_CMD_POWER_CTRL2, 			1, 0x10,
		ILI9341_CMD_VCOM_CTRL1, 			2, 0x3E, 0x28,
		ILI9341_CMD_VCOM_CTRL2, 			1, 0x86,
		ILI9341_CMD_MEM_ACCESS_CTRL, 		1, ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_CMD_PIXEL_FORMAT_SET, 		1, (ILI9341_PF_16BIT << 4) | ILI9341_PF_16BIT,
		ILI9341_CMD_FRAME_RATE_CTRL_NORMAL,	2, 0x00, 0x18,
		ILI9341_CMD_DISP_FUNC_CTRL, 		3, 0x08, 0x82, 0x27,
		ILI9341_CMD_NOP
	};
	static const uint8_t lcd_rotation[] = {
		ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR
	};
#elif LCD_TYPE == LCD_3_2_INCH
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
	static const uint8_t lcd_rotation[] = {
		ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MV | ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR
	};
#endif
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	pinMode(PIN_LCD_RD, OUTPUT);
	pinMode(PIN_LCD_WR, OUTPUT);
	pinMode(PIN_LCD_RS, OUTPUT);
	pinMode(PIN_LCD_CS, OUTPUT);
	pinMode(PIN_LCD_RST, OUTPUT);
#if LCD_TYPE == LCD_2_4_INCH
	for (int i = 2; i < 10; i++) pinMode(i, OUTPUT);
#elif LCD_TYPE == LCD_3_2_INCH
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
#endif

	digitalWrite(PIN_LCD_RD, HIGH);
	digitalWrite(PIN_LCD_WR, HIGH);
	digitalWrite(PIN_LCD_RS, HIGH);
	digitalWrite(PIN_LCD_CS, HIGH);
	digitalWrite(PIN_LCD_RST, HIGH);

	Serial.begin(115200); //Doesn't matter speed

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
	s_uart_log.act_send_data = actLogSendData;
    s_uart_log.act_receive_data = actLogReceiveData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;			// 2EA

#if LCD_TYPE == LCD_2_4_INCH
	if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_8BIT_I)) return;

	if (!pifTouchScreen_Init(&g_touch_screen, PIF_ID_AUTO, &g_ili9341.parent, 136, 907, 139, 942)) return;
	if (!pifTouchScreen_AttachAction(&g_touch_screen, actTouchPosition, actTouchPressure)) return;
#elif LCD_TYPE == LCD_3_2_INCH
    if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_16BIT_I)) return;
    g_ili9341.parent.act_backlight = actLcdBackLight;

    if (!pifSpiPort_Init(&g_spi_port, PIF_ID_AUTO, 1, 16)) return;
    g_spi_port.act_transfer = actTransfer;

    if (!pifTsc2046_Init(&g_tsc2046, PIF_ID_AUTO, &g_ili9341.parent, 3761, 229, 3899, 420, &g_spi_port, actPen)) return;
#endif
    if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, actLcdReadCmd, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) return;
    pifIli9341_Setup(&g_ili9341, lcd_setup, lcd_rotation);

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
