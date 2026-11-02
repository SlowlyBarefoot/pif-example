#ifndef APP_PORT_H
#define APP_PORT_H


#include "communication/pif_usb_cdc.h"
#include "core/pif_log.h"
#include "display/pif_led.h"


#define TASK_SIZE				8


extern PifLed g_led_l;
extern PifUsbCdc g_cdc;				// log and command line


uint32_t actMicros();
BOOL portSetup();
BOOL appSetup();

// Called from usbd_cdc_if.c
BOOL portCdcReceive(uint8_t *p_data, uint32_t length);
void portCdcTxComplete(void);
void portCdcLineState(uint16_t state);

// Defined in usbd_cdc_if.c
void CDC_ArmReceive(void);


#endif	// APP_PORT_H
