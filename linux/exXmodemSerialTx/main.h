#ifndef MAIN_H_
#define MAIN_H_


#include "pifComm.h"


void actLogPrint(char *pcString);
uint16_t actSerialSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerialReceiveData(PIF_stComm *pstComm, uint8_t *pucData);


#endif /* MAIN_H_ */
