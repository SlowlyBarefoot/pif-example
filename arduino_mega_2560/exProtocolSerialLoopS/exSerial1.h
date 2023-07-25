#ifndef EXSERIAL1_H_
#define EXSERIAL1_H_


#include "filter/pif_noise_filter_bit.h"
#include "sensor/pif_sensor_switch.h"


#define SWITCH_COUNT          	2


typedef struct {
	PifSensorSwitch stPushSwitch;
	uint8_t ucDataCount;
	uint8_t ucData[8];
	PifNoiseFilterBit stPushSwitchFilter;
} ProtocolTest;


extern PifUart g_serial1;

extern ProtocolTest g_stProtocolTest[SWITCH_COUNT];


BOOL exSerial1_Setup();


#endif /* EXSERIAL1_H_ */
