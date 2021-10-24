#ifndef MAIN_H_
#define MAIN_H_


#include "pif_comm.h"


uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
uint16_t actSerialSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerialReceiveData(PifComm *pstComm, uint8_t *pucData);


#endif /* MAIN_H_ */
