#ifndef PIF_CONF_H
#define PIF_CONF_H


// -------- pif Configuration --------------------

//#define PIF_DEBUG

//#define PIF_STATIC_INLINE			static inline

//#define PIF_WEAK							__attribute__ ((weak))


// -------- pifBasic -----------------------------

// pifGraphics_Printf의 %f를 쓰려면 필요하다.
#define PIF_USE_PRINTF_FLOAT


// -------- pifCollectSignal ---------------------

//#define PIF_COLLECT_SIGNAL


// -------- pifGraphics --------------------------

// 흑백 패널이라 0이 꺼짐, 그 외가 켜짐이므로 16으로 둔다. 32는 PifColor가 4바이트가 되어 손해다.
#define PIF_COLOR_DEPTH 					16

//#define PIF_GRAPHICS_PRINTF_SIZE			32

// 라이브러리의 기본 폰트(5x8)를 빼고 앱의 폰트만 쓸 때 정의한다.
//#define PIF_GRAPHICS_NO_DEFAULT_FONT


// -------- pifLog -------------------------------

//#define PIF_NO_LOG
#define PIF_LOG_COMMAND

//#define PIF_LOG_LINE_SIZE					80


// -------- pifSsd1306 ---------------------------

// pifSsd1306_Begin이 마지막에 charge pump 안정화를 기다리는 시간
//#define PIF_SSD1306_BEGIN_DELAY_MS		100


// -------- pifTask ------------------------------

//#define PIF_USE_TASK_STATISTICS


// -------- pifTimer -----------------------------

//#define PIF_PWM_MAX_DUTY					1000


#endif  // PIF_CONF_H
