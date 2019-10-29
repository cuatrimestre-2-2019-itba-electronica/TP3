//
// Created by Matias Pierdominici on 10/24/2019.
//

#include "demCircBuffer.h"

void demCircBufferInit(demCircBufferBuff * dataS, int32_t * buffer,uint16_t buffSize,uint8_t ofestPtr,bool initCero){
    dataS->data=buffer;
    dataS->dataSize=buffSize;
    dataS->head=0;
    dataS->read=0;
    dataS->off=ofestPtr;
    dataS->offPtr=dataS->dataSize - dataS->off;
    if(initCero){
        for(int i=0;i<buffSize;i++){
            buffer[i]=0;
        }
    }else{
    	for(int i=0;i<buffSize;i++){
    		buffer[i]=1;
    	}
    }


}

void demCircBufferApend(demCircBufferBuff * dataS,int32_t data2apend){
    (dataS->data)[dataS->head]=data2apend;//agrego el dato al buffer
    dataS->head++;
    dataS->head%=dataS->dataSize; //reinicio el buffer
}

uint16_t demCircBufferAmauntData(demCircBufferBuff * dataS){
    if(dataS->head >= dataS->read){
        return dataS->head - dataS->read;
    }else{
        return dataS->dataSize - dataS->read + dataS->head;
    }
}

bool demCircBufferGetData(demCircBufferBuff * dataS,int32_t * dato,int32_t * datoOff){
    *dato=(dataS->data)[dataS->read];
    *datoOff=(dataS->data)[dataS->offPtr];
    dataS->read++;
    dataS->offPtr++;
    dataS->read%=dataS->dataSize;
    dataS->offPtr%=dataS->dataSize;
}

void demCircBufferGetDataBetweenPtr(demCircBufferBuff *dataS, int32_t *dato) {
    uint16_t offT=0;
    offT=dataS->offPtr;

    for(int i=0;i<dataS->off+1;i++){
        dato[dataS->off-i]=dataS->data[offT];
        offT++;
        offT%=dataS->dataSize;
    }

    int32_t t1;
    int32_t t2;
    demCircBufferGetData(dataS,&t1,&t2);

}

