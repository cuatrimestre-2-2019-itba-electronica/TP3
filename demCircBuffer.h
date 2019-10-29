//
// Created by Matias Pierdominici on 10/24/2019.
//

#ifndef DEMODULADORFSK_DEMCIRCBUFFER_H
#define DEMODULADORFSK_DEMCIRCBUFFER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct demCircBufferBuff {
    int32_t * data;
    uint16_t dataSize;
    uint16_t head;
    uint16_t read;
    uint8_t off;
    uint16_t offPtr;
} demCircBufferBuff;

void demCircBufferInit(demCircBufferBuff * dataS, int32_t * buffer,uint16_t buffSize,uint8_t ofestPtr,bool initCero);

void demCircBufferApend(demCircBufferBuff * dataS,int32_t data2apend);

uint16_t demCircBufferAmauntData(demCircBufferBuff * dataS);

bool demCircBufferGetData(demCircBufferBuff * dataS,int32_t * dato,int32_t * datoOff);

void demCircBufferGetDataBetweenPtr(demCircBufferBuff * dataS,int32_t * dato);

#endif //DEMODULADORFSK_DEMCIRCBUFFER_H
