#ifndef APP_PORT_H
#define APP_PORT_H


#include "actuator/pif_servo.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "input/pif_joystick.h"


#define TASK_SIZE				10
#define TIMER_1MS_SIZE			1

#define SERVO_COUNT				1
#define SERVO_PERIOD			20		// 20ms, the period of TIM3
#define SERVO_PINS				"D12 (PA6)"

#define JOYSTICK_PINS			"VRx A0 (PA0), VRy A1 (PA1)"
#define JOYSTICK_RAW_MAX		4095	// 12-bit ADC

#define MONITOR_PERIOD			200		// ms between the lines of the monitor


extern PifLed g_led_l;
extern PifServo g_servo;
extern PifJoystick g_joystick;
extern PifTimerManager g_timer_1ms;


uint32_t actMicros();
BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
