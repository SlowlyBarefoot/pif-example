// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_max31855_H_
#define _ex_max31855_H_
#include "Arduino.h"
//add your includes for the project ex_max31855 here

#include "core/pif_comm.h"
#include "core/pif_spi.h"

//end of add your includes here


//add your function definitions for the project ex_max31855 here

uint16_t actLogSendData(PifComm *p_owner, uint8_t *p_buffer, uint16_t size);
void actLedLState(PifId id, uint32_t state);
BOOL actTransfer(PifId id, uint8_t* p_write, uint8_t* p_read, uint16_t size);

//Do not add code below this line
#endif /* _ex_max31855_H_ */
