// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_storage_H_
#define _ex_storage_H_
#include "Arduino.h"
//add your includes for the project ex_storage here
#include <DueFlashStorage.h>

#include "pif_comm.h"

//end of add your includes here

#define STORAGE_SECTOR_SIZE		IFLASH1_PAGE_SIZE
#define STORAGE_VOLUME			IFLASH1_SIZE

//add your function definitions for the project ex_storage here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifComm *pstOwner, uint8_t *pucData);
void actLedL(SWITCH sw);
BOOL actStorageRead(uint8_t* dst, uint32_t src, size_t size, void* p_issuer);
BOOL actStorageWrite(uint32_t dst, uint8_t* src, size_t size, void* p_issuer);

//Do not add code below this line
#endif /* _ex_storage_H_ */
