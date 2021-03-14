#ifndef MAIN_H_
#define MAIN_H_


#include "pifRingBuffer.h"


void actLogPrint(char *pcString);
BOOL actSerialSendData(PIF_stRingBuffer *pstBuffer);
void actSerialReceiveData(PIF_stRingBuffer *pstBuffer);


#endif /* MAIN_H_ */
