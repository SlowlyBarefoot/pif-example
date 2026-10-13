#ifndef EXCAN0_H_
#define EXCAN0_H_


#include "communication/pif_can.h"
#include "sensor/pif_sensor_switch.h"


#define PIF_ID_SWITCH			0x100

#define SWITCH_COUNT          	2


typedef struct {
	PifSensorSwitch push_switch;
	uint8_t data_count;
	uint8_t data[8];
} LinkTest;


extern PifCan g_can0;

extern LinkTest g_link_test[SWITCH_COUNT];


BOOL exCan0_Setup();


#endif /* EXCAN0_H_ */
