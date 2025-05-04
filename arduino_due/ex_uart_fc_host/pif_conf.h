#ifndef PIF_CONF_H
#define PIF_CONF_H


// -------- pifCollectSignal ---------------------

//#define PIF_COLLECT_SIGNAL

// -------- pif Configuration --------------------

//#define PIF_DEBUG

//#define PIF_INLINE
#define PIF_INLINE                      	inline
//#define PIF_INLINE                      	__inline

//#define PIF_WEAK							__attribute__ ((weak))

// -------- pifGpsNmea ---------------------------

//#define PIF_GPS_NMEA_VALUE_SIZE			32
//#define PIF_GPS_NMEA_TEXT_SIZE			64

// -------- pifKeypad ----------------------------

//#define PIF_KEYPAD_DEFAULT_HOLD_TIME		100
//#define PIF_KEYPAD_DEFAULT_LONG_TIME		1000
//#define PIF_KEYPAD_DEFAULT_DOUBLE_TIME	300

// -------- pifLog -------------------------------

//#define PIF_NO_LOG
#define PIF_LOG_COMMAND

//#define PIF_LOG_LINE_SIZE					80

// -------- pifTask ------------------------------

//#define PIF_USE_TASK_STATISTICS

// -------- pifTimer -----------------------------

//#define PIF_PWM_MAX_DUTY					1000


#endif  // PIF_CONF_H
