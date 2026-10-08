#ifndef EXSERIAL1_H_
#define EXSERIAL1_H_


#include "communication/pif_uart.h"
#include "sensor/pif_sensor_switch.h"


#define PIF_ID_SWITCH			0x100

#define SWITCH_COUNT          	2


typedef struct {
	PifSensorSwitch push_switch;
	uint8_t data_count;
	uint8_t data[8];
} LinkTest;


extern PifUart g_serial1;

extern LinkTest g_link_test[SWITCH_COUNT];


BOOL exSerial1_Setup();


#endif /* EXSERIAL1_H_ */
