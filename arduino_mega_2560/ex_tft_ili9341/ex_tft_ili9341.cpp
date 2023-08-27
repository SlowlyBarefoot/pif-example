// Do not remove the include below
#include "ex_tft_ili9341.h"
#include "linker.h"

#include <MsTimer2.h>


#define PIN_LED_L				13

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS		A3 // Chip Select goes to Analog 3
#define LCD_RS		A2 // Command/Data goes to Analog 2
#define LCD_WR		A1 // LCD Write goes to Analog 1
#define LCD_RD		A0 // LCD Read goes to Analog 0

#define LCD_RST		A4 // Can alternately just connect to Arduino's reset pin

#define TS_YP 		A1  // must be an analog pin, use "An" notation!
#define TS_XP 		A2  // must be an analog pin, use "An" notation!
#define TS_YM 		7   // can be a digital pin
#define TS_XM 		6   // can be a digital pin

#define EMASK       0x38
#define GMASK       0x20
#define HMASK       0x78

#define GPIO_OUTPUT() { \
	DDRH |=  HMASK; DDRE |=  EMASK; DDRG |=  GMASK; \
}
#define GPIO_INPUT() { \
	DDRH &= ~HMASK; DDRE &= ~EMASK; DDRG &= ~GMASK; \
}

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

static BOOL actLogReceiveData(PifUart *p_uart, uint8_t *pucData)
{
	int rxData;

	(void)p_uart;

	rxData = Serial.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

static void actLcdReset()
{
	digitalWrite(LCD_RST,LOW);
	delay(100);
	digitalWrite(LCD_RST,HIGH);
	delay(50);
}

static void actLcdChipSelect(SWITCH sw)
{
	digitalWrite(LCD_CS, sw ? LOW : HIGH);
}

static uint16_t _LcdReadBus()
{
	uint32_t t;
	uint16_t d;

	digitalWrite(LCD_RD, LOW);
	delayMicroseconds(1);
	t = PINH;
	d = ((t << 3) & 0xC0) | ((t >> 5) & 0x03);
	t = PINE;
	d |= ((t >> 2) & 0xC) | ((t << 2) & 0x20);
	t = PING;
	d |= ((t >> 1) & 0x10);
	digitalWrite(LCD_RD, HIGH);
	return d;
}

static void _LcdWritBus(uint16_t d)
{
	PORTH &= ~HMASK;
	PORTH |= ((d & 0xC0) >> 3) | ((d & 0x3) << 5);
	PORTE &= ~EMASK;
	PORTE |= ((d & 0xC) << 2) | ((d & 0x20) >> 2);
	PORTG &= ~GMASK;
	PORTG |= (d & 0x10) << 1;
	digitalWrite(LCD_WR, LOW);
	digitalWrite(LCD_WR, HIGH);
}

static void actLcdReadCmd(PifIli9341Cmd cmd, uint16_t* p_data, uint32_t len)
{
	digitalWrite(LCD_RS, LOW);
	_LcdWritBus(cmd);

	if (len) {
		digitalWrite(LCD_RS, HIGH);
		GPIO_INPUT();
		delayMicroseconds(1);
		for (uint32_t i = 0; i < len; i++) {
			p_data[i] = _LcdReadBus();
		}
		GPIO_OUTPUT();
	}
}

static void actLcdWriteCmd(PifIli9341Cmd cmd, uint16_t* p_data, uint32_t len)
{
	digitalWrite(LCD_RS, LOW);
	_LcdWritBus(cmd);

	if (len) {
		digitalWrite(LCD_RS, HIGH);
		for (uint32_t i = 0; i < len; i++) {
			_LcdWritBus(p_data[i]);
		}
	}
}

static void actLcdWriteData(uint16_t* p_data, uint32_t len)
{
	digitalWrite(LCD_RS, HIGH);
	for (uint32_t i = 0; i < len; i++) {
		_LcdWritBus(p_data[i]);
	}
}

static void actLcdWriteRepeat(uint16_t* p_data, uint8_t size, uint32_t len)
{
	uint8_t p = 0;

	digitalWrite(LCD_RS, HIGH);
	for (uint32_t i = 0; i < size * len; i++) {
		_LcdWritBus(p_data[p]);
		p++;
		if (p >= size) p = 0;
	}
}

static void actTouchPosition(PifTouchScreen* p_owner, int16_t* x, int16_t* y)
{
    uint8_t i;

	(void)p_owner;

	digitalWrite(TS_YM, LOW);
	pinMode(TS_YP, INPUT);
	pinMode(TS_YM, INPUT);

	digitalWrite(TS_XP, HIGH);
	digitalWrite(TS_XM, LOW);

    *x = analogRead(TS_YP);

	pinMode(TS_YP, OUTPUT);
	pinMode(TS_YM, OUTPUT);

	digitalWrite(TS_XM, LOW);
	pinMode(TS_XP, INPUT);
	pinMode(TS_XM, INPUT);

	digitalWrite(TS_YP, HIGH);
    digitalWrite(TS_YM, LOW);

    *y = analogRead(TS_XP);

    pinMode(TS_XM, OUTPUT);
    pinMode(TS_XP, OUTPUT);
}

static BOOL actTouchPressure(PifTouchScreen* p_owner)
{
	int16_t z;

	(void)p_owner;

	// Set X+ to ground
	digitalWrite(TS_XM, LOW);

	// Set Y- to VCC
	digitalWrite(TS_YM, HIGH);

	// Hi-Z X- and Y+
    digitalWrite(TS_XP, LOW);
	digitalWrite(TS_YP, LOW);
    pinMode(TS_XP, INPUT);
	pinMode(TS_YP, INPUT);

	int z1 = analogRead(TS_XP);
	int z2 = analogRead(TS_YP);

    if (z1 < 10) z = 0;
    else z = 1023 - (z2 - z1);

    pinMode(TS_YP, OUTPUT);
    pinMode(TS_XP, OUTPUT);

    return z > 150;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	const uint16_t lcd_setup[] = {
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
		ILI9341_CMD_PIXEL_FORMAT_SET, 		1, 0x55,
		ILI9341_CMD_FRAME_RATE_CTRL_NORMAL,	2, 0x00, 0x18,
		ILI9341_CMD_DISP_FUNC_CTRL, 		3, 0x08, 0x82, 0x27,
		ILI9341_CMD_NOP
	};
	static const uint16_t lcd_rotation[] = {
		ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR
	};
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	pinMode(LCD_RD, OUTPUT);
	pinMode(LCD_WR, OUTPUT);
	pinMode(LCD_RS, OUTPUT);
	pinMode(LCD_CS, OUTPUT);
	pinMode(LCD_RST, OUTPUT);
	for (int i = 2; i < 10; i++) pinMode(i, OUTPUT);

	digitalWrite(LCD_RD, HIGH);
	digitalWrite(LCD_WR, HIGH);
	digitalWrite(LCD_RS, HIGH);
	digitalWrite(LCD_CS, HIGH);
	digitalWrite(LCD_RST, HIGH);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
	s_uart_log.act_send_data = actLogSendData;
    s_uart_log.act_receive_data = actLogReceiveData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;			// 2EA

	if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_PARALLEL_8BIT)) return;

	if (!pifTouchScreen_Init(&g_touch_screen, PIF_ID_AUTO, &g_ili9341.parent, 136, 907, 139, 942)) return;
	if (!pifTouchScreen_AttachAction(&g_touch_screen, actTouchPosition, actTouchPressure)) return;
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
