#ifndef APP_PORT_H
#define APP_PORT_H


#include "actuator/pif_servo.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


#define TASK_SIZE				8
#define TIMER_1MS_SIZE			1

#define SERVO_COUNT				3
#define SERVO_PERIOD			20		// 20ms, the period of TIM3
#define SERVO_PINS				"D12 (PA6), D11 (PA7) and A3 (PB0)"

// Pulses the commands take. TIM3 could give up to its period, but no RC servo takes pulses beyond these.
#define SERVO_PULSE_MIN			400
#define SERVO_PULSE_MAX			2600


extern PifLed g_led_l;
extern PifServo g_servo;
extern PifTimerManager g_timer_1ms;


uint32_t actMicros();
BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
