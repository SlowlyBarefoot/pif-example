// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_gps_serial_ublox_H_
#define _ex_gps_serial_ublox_H_
#include "Arduino.h"
//add your includes for the project ex_gps_serial_ublox here

#include "communication/pif_uart.h"

//end of add your includes here


//add your function definitions for the project ex_gps_serial_ublox here

void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifUart *pstOwner, uint8_t *pucData);
void actGpsSetBaudrate(PifUart *p_owner, uint32_t baudrate);
uint16_t actGpsSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actGpsReceiveData(PifUart *pstOwner, uint8_t *pucData);

//Do not add code below this line
#endif /* _ex_gps_serial_ublox_H_ */
