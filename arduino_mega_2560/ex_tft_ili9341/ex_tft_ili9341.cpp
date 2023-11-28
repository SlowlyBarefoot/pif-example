// Do not remove the include below
#include "ex_tft_ili9341.h"
#include "linker.h"

#include <MsTimer2.h>
#if LCD_TYPE == LCD_2_2_INCH_SPI || LCD_TYPE == LCD_3_2_INCH
	#include <SPI.h>
#endif


#define PIN_LED_L				13

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define PIN_LCD_CS				A3 // Chip Select goes to Analog 3
#define PIN_LCD_RS				A2 // Command/Data goes to Analog 2
#define PIN_LCD_RST				A4 // Can alternately just connect to Arduino's reset pin

#if LCD_TYPE == LCD_2_2_INCH_SPI

	#define PIN_LCD_BL			7

#else

	#define PIN_LCD_WR			A1 // LCD Write goes to Analog 1
	#define PIN_LCD_RD			A0 // LCD Read goes to Analog 0

	#define PIN_TS_YP 			A1  // must be an analog pin, use "An" notation!
	#define PIN_TS_XP 			A2  // must be an analog pin, use "An" notation!
	#define PIN_TS_YM 			7   // can be a digital pin
	#define PIN_TS_XM 			6   // can be a digital pin

	#define EMASK    			0x38
	#define GMASK   		    0x20
	#define HMASK   		    0x78

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
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	if (p_rate) *p_rate = 100 * Serial.available() / SERIAL_RX_BUFFER_SIZE;
	return i;
}

static void actLcdReset()
{
	digitalWrite(PIN_LCD_RST, HIGH);
	delay(5);
	digitalWrite(PIN_LCD_RST, LOW);
	delay(15);
	digitalWrite(PIN_LCD_RST, HIGH);
	delay(15);
}

static void actLcdChipSelect(SWITCH sw)
{
	digitalWrite(PIN_LCD_CS, sw ? LOW : HIGH);
}

#if LCD_TYPE == LCD_2_2_INCH_SPI

static void _LcdWritBus(uint32_t d)
{
	SPI.transfer(d);
}

#else

static uint32_t _LcdReadBus()
{
	uint32_t t, d;

	digitalWrite(PIN_LCD_RD, LOW);
	delayMicroseconds(1);
	t = PINH;
	d = ((t << 3) & 0xC0) | ((t >> 5) & 0x03);
	t = PINE;
	d |= ((t >> 2) & 0xC) | ((t << 2) & 0x20);
	t = PING;
	d |= ((t >> 1) & 0x10);
	digitalWrite(PIN_LCD_RD, HIGH);
	return d;
}

static void _LcdWritBus(uint32_t d)
{
	PORTH &= ~HMASK;
	PORTH |= ((d & 0xC0) >> 3) | ((d & 0x3) << 5);
	PORTE &= ~EMASK;
	PORTE |= ((d & 0xC) << 2) | ((d & 0x20) >> 2);
	PORTG &= ~GMASK;
	PORTG |= (d & 0x10) << 1;
	digitalWrite(PIN_LCD_WR, LOW);
	digitalWrite(PIN_LCD_WR, HIGH);
}

static void actLcdReadCmd(PifTftLcdCmd cmd, uint32_t* p_data, uint32_t size)
{
	digitalWrite(PIN_LCD_RS, LOW);
	_LcdWritBus(cmd);

	if (size) {
		digitalWrite(PIN_LCD_RS, HIGH);
		DDRH &= ~HMASK; DDRE &= ~EMASK; DDRG &= ~GMASK;
		delayMicroseconds(1);
		for (uint32_t i = 0; i < size; i++) {
			p_data[i] = _LcdReadBus();
		}
		DDRH |=  HMASK; DDRE |=  EMASK; DDRG |=  GMASK;
	}
}

#endif

static void actLcdWriteCmd(PifTftLcdCmd cmd, uint32_t* p_data, uint32_t size)
{
	*(portOutputRegister(digitalPinToPort(PIN_LCD_RS))) &= ~digitalPinToBitMask(PIN_LCD_RS);
//	digitalWrite(PIN_LCD_RS, LOW);
	_LcdWritBus(cmd);

	if (size) {
		*(portOutputRegister(digitalPinToPort(PIN_LCD_RS))) |= digitalPinToBitMask(PIN_LCD_RS);
//		digitalWrite(PIN_LCD_RS, HIGH);
		for (uint32_t i = 0; i < size; i++) {
			_LcdWritBus(p_data[i]);
		}
	}
}

static void actLcdWriteData(uint32_t* p_data, uint32_t size)
{
	*(portOutputRegister(digitalPinToPort(PIN_LCD_RS))) |= digitalPinToBitMask(PIN_LCD_RS);
//	digitalWrite(PIN_LCD_RS, HIGH);
	for (uint32_t i = 0; i < size; i++) {
		_LcdWritBus(p_data[i]);
	}
}

static void actLcdWriteRepeat(uint32_t* p_data, uint8_t size, uint32_t len)
{
	uint8_t p = 0;

	*(portOutputRegister(digitalPinToPort(PIN_LCD_RS))) |= digitalPinToBitMask(PIN_LCD_RS);
//	digitalWrite(PIN_LCD_RS, HIGH);
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

#endif

#if LCD_TYPE == LCD_2_2_INCH_SPI

static void actLcdBackLight(uint8_t level)
{
	analogWrite(PIN_LCD_BL, (uint16_t)level * 255 / 100);
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
	static PifUart s_uart_log;
	int line;

	pinMode(PIN_LED_L, OUTPUT);

	pinMode(PIN_LCD_RS, OUTPUT);
	pinMode(PIN_LCD_CS, OUTPUT);
	pinMode(PIN_LCD_RST, OUTPUT);

#if LCD_TYPE == LCD_2_2_INCH_SPI

	pinMode(PIN_LCD_BL, OUTPUT);

	analogWrite(PIN_LCD_BL, 0x7F);

    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz (half speed)
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);

#elif LCD_TYPE == LCD_2_4_INCH

	pinMode(PIN_LCD_RD, OUTPUT);
	pinMode(PIN_LCD_WR, OUTPUT);
	for (int i = 2; i < 10; i++) pinMode(i, OUTPUT);

	digitalWrite(PIN_LCD_RD, HIGH);
	digitalWrite(PIN_LCD_WR, HIGH);
#endif

	digitalWrite(PIN_LCD_RS, HIGH);
	digitalWrite(PIN_LCD_CS, HIGH);
	digitalWrite(PIN_LCD_RST, HIGH);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) { line = __LINE__; goto fail; }

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) { line = __LINE__; goto fail; }					// 1ms
	s_uart_log.act_send_data = actLogSendData;
    s_uart_log.act_receive_data = actLogReceiveData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) { line = __LINE__; goto fail; }

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) { line = __LINE__; goto fail; }			// 2EA

#if LCD_TYPE == LCD_2_2_INCH_SPI
	if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_8BIT_I)) { line = __LINE__; goto fail; }
    g_ili9341.parent.act_backlight = actLcdBackLight;

    if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, NULL, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
#elif LCD_TYPE == LCD_2_4_INCH
	if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_8BIT_I)) { line = __LINE__; goto fail; }

	if (!pifTouchScreen_Init(&g_touch_screen, PIF_ID_AUTO, &g_ili9341.parent, 136, 907, 139, 942)) { line = __LINE__; goto fail; }
	if (!pifTouchScreen_AttachAction(&g_touch_screen, actTouchPosition, actTouchPressure)) { line = __LINE__; goto fail; }
    if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, actLcdReadCmd, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
#endif
    pifIli9341_Setup(&g_ili9341, lcd_setup, lcd_rotation);

	if (!appSetup()) { line = __LINE__; goto fail; }

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
