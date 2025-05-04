// Do not remove the include below
#include "ex_tft_ssd1963.h"
#include "linker.h"

#include <SPI.h>


#define PIN_LED_L				13

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define PIN_LCD_CS				A3 // Chip Select goes to Analog 3
#define PIN_LCD_RS				A2 // Command/Data goes to Analog 2
#define PIN_LCD_RST				A4 // Can alternately just connect to Arduino's reset pin
#define PIN_LCD_WR				A1 // LCD Write goes to Analog 1
#define PIN_LCD_RD				A0 // LCD Read goes to Analog 0
#define PIN_LCD_BL				7

#define PIN_TS_INT				2
#define PIN_TS_CS				3
#define PIN_TS_PEN				4

#define TASK_SIZE				10
#define TIMER_1MS_SIZE			3

#define UART_LOG_BAUDRATE		115200

#define HDP						(480 - 1)
#define VDP						(272 - 1)

#define HT						531
#define HPS						43
#define HPW						10
#define LPS						8

#define VT						288
#define VPS						12
#define VPW						10
#define FPS						4


const uint8_t lcd_setup[] = {
	SSD1963_CMD_SET_PLL_MN, 		3, 0x023, 0x02, 0x04,
	SSD1963_CMD_SET_PLL,			1, 0x01,
	TFT_SETUP_DELAY_MS,				10,
	SSD1963_CMD_SET_PLL,	 		1, 0x03,
	TFT_SETUP_DELAY_MS,				10,
	SSD1963_CMD_SOFT_RESET, 		0,
	TFT_SETUP_DELAY_MS,				100,
	SSD1963_CMD_SET_LSHIFT_FREQ, 	3, 0x01, 0x33, 0x32,
	SSD1963_CMD_SET_LCD_MODE,		7, 0x20, 0x00, (HDP >> 8) & 0xFF, HDP & 0xFF, (VDP >> 8) & 0xFF, VDP & 0xFF, 0x00,
	SSD1963_CMD_SET_HORI_PERIOD,	8, (HT >> 8) & 0xFF, HT & 0xFF, (HPS >> 8) & 0xFF, HPS & 0xFF, HPW, (LPS >> 8) & 0xFF, LPS & 0xFF, 0x00,
	SSD1963_CMD_SET_VERT_PERIOD, 	7, (VT >> 8) & 0xFF, VT & 0xFF, (VPS >> 8) & 0xFF, VPS & 0xFF, VPW, (FPS >> 8) & 0xFF, FPS & 0xFF,
	SSD1963_CMD_SET_GPIO_VALUE,		1, 0x01,
	SSD1963_CMD_SET_GPIO_CONF,		2, 0x01, 0x01,
	SSD1963_CMD_SET_PIX_DATA_IF,	1, SSD1963_PF_16BIT_565,
	SSD1963_CMD_ENTER_INVERT_MODE,	0,
	SSD1963_CMD_SET_POST_PROC,		4, 0x80, 0x80, 0x80, 0x01,
	SSD1963_CMD_NOP
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
	PIO_Set(PIOC, ((d & 0x00FF) << 2) | ((d & 0xFF00) << 4));
	PIO_Clear(PIOC, ((~d & 0x00FF) << 2) | ((~d & 0xFF00) << 4));
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
		PIOC->PIO_PUDR |= 0x000FF3FC; PIOC->PIO_IFDR |= 0x000FF3FC;	PIOC->PIO_ODR |= 0x000FF3FC; PIOC->PIO_PER |= 0x000FF3FC;
		delayMicroseconds(1);
		for (uint32_t i = 0; i < size; i++) {
			p_data[i] = _LcdReadBus();
		}
		PIOC->PIO_MDDR |= 0x000FF3FC; PIOC->PIO_OER |= 0x000FF3FC; PIOC->PIO_PER |=  0x000FF3FC;
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
	pinMode(PIN_TS_INT, INPUT_PULLUP);
	pinMode(PIN_TS_CS, OUTPUT);
	pinMode(PIN_TS_PEN, INPUT_PULLUP);
	pinMode(PIN_LCD_BL, OUTPUT);

	analogWrite(PIN_LCD_BL, 0xFF);

	SPI.begin();
//	SPI.setClockDivider(SPI_CLOCK_DIV16);
	digitalWrite(SS, HIGH);

	digitalWrite(PIN_LCD_RD, HIGH);
	digitalWrite(PIN_LCD_WR, HIGH);
	digitalWrite(PIN_LCD_RS, HIGH);
	digitalWrite(PIN_LCD_CS, HIGH);
	digitalWrite(PIN_LCD_RST, HIGH);

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) { line = __LINE__; goto fail; }

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachTxTask(&s_uart_log, TM_EXTERNAL_ORDER, 0, NULL)) { line = __LINE__; goto fail; }
    if (!pifUart_AttachRxTask(&s_uart_log, TM_PERIOD, 200000, NULL)) { line = __LINE__; goto fail; }				// 200ms
    s_uart_log.act_receive_data = actLogReceiveData;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) { line = __LINE__; goto fail; }										// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) { line = __LINE__; goto fail; }			// 2EA

    if (!pifSsd1963_Init(&g_ssd1963, PIF_ID_AUTO)) { line = __LINE__; goto fail; }
    g_ssd1963.parent.act_backlight = actLcdBackLight;

    if (!pifSpiPort_Init(&g_spi_port, PIF_ID_AUTO, 1)) { line = __LINE__; goto fail; }
    g_spi_port.act_transfer = actTransfer;

    if (!pifTsc2046_Init(&g_tsc2046, PIF_ID_AUTO, &g_ssd1963.parent, 3761, 229, 3899, 420, &g_spi_port, NULL, actPen)) { line = __LINE__; goto fail; }
    if (!pifSsd1963_AttachActParallel(&g_ssd1963, actLcdReset, actLcdChipSelect, actLcdReadCmd, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
    if (!pifSsd1963_Setup(&g_ssd1963, lcd_setup, NULL)) { line = __LINE__; goto fail; }

	if (!appSetup()) { line = __LINE__; goto fail; }

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***          ex_tft_ssd1963          ***\n");
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
