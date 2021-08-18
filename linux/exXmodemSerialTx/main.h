#ifndef MAIN_H_
#define MAIN_H_


#include "pifComm.h"


uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
uint16_t actSerialSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerialReceiveData(PIF_stComm *pstComm, uint8_t *pucData);


#endif /* MAIN_H_ */
