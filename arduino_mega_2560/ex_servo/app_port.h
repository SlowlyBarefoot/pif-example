#ifndef APP_PORT_H
#define APP_PORT_H


#include "actuator/pif_servo.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


#define TASK_SIZE				8
#define TIMER_1MS_SIZE			1

#define SERVO_COUNT				3
#define SERVO_PERIOD			20		// 20ms, the frame of the Servo library
#define SERVO_PINS				"9, 10 and 11"

// Pulses the Servo library is attached with, which it holds every pulse within. pifServo_SetRange() can go as far.
#define SERVO_PULSE_MIN			400
#define SERVO_PULSE_MAX			2600


extern PifLed g_led_l;
extern PifServo g_servo;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
