//
// Created by Matias Pierdominici on 10/25/2019.
//

#include "demFskV1.h"
#include "demCircBuffer.h"
#include <stdint.h>
#include "fdacoefs.h"
#include "gpio.h"
#include "board.h"
#include <stdio.h>
#include <stdbool.h>
#include "adc.h"

#define DEM_FSK_V1_SIGNAL_OFSET 32767

#define DEM_FSK_V1_XSIZE 100
#define DEM_FSK_V1_YSIZE 100
#define DEM_FSK_V1_TAP_XN 5
#define DEM_FSK_V1_TAP_YN 18
#define DEM_FSK_V1_TAP_COMP_N 10
#define DEM_FSK_V1_COMP_SIZE 100

typedef enum{DEM_IDLE,DEM_GET_BIT,DEM_PAR}states;



demCircBufferBuff xN; //senala generada por el adc
int32_t xNArray[DEM_FSK_V1_XSIZE];

demCircBufferBuff yN;//senal xn * xn-5
int32_t yNArray[DEM_FSK_V1_YSIZE];

demCircBufferBuff CompN;//senal despues del comparador
int32_t CompNArray[DEM_FSK_V1_COMP_SIZE];

uint16_t adcMeasData=0; //almaceno la muestra que midio el adc
bool hayMedicion=false; //si es true indica que hay muestra para procesar

void demFskV1AdcCallback(void);
static float aplyFilter(int32_t * dato);
void demFskV1AddSample(int32_t sample) {
    int32_t d=0;
    int32_t  dTap=0;
    int32_t tempBuf[30];
    demCircBufferApend(&xN,sample);
    if(demCircBufferAmauntData(&xN)){
        demCircBufferGetData(&xN,&d,&dTap);//levanto xn y xn-t
        //printf("%d \n",d*dTap);
        demCircBufferApend(&yN,d*dTap);//guardo yn en el buffer
        demCircBufferGetDataBetweenPtr(&yN,tempBuf);

        if(aplyFilter(tempBuf)>0){//salida del comparador
        	gpioWrite(PIN_LED_RED, true);
        	demCircBufferApend(&CompN,1);
        }else{
        	gpioWrite(PIN_LED_RED, false);
        	demCircBufferApend(&CompN,0);
        }

    }



}

void demFskV1Init(void) {
    demCircBufferInit(&xN,xNArray,DEM_FSK_V1_XSIZE,DEM_FSK_V1_TAP_XN,true);
    demCircBufferInit(&yN,yNArray,DEM_FSK_V1_YSIZE,DEM_FSK_V1_TAP_YN,true);
    demCircBufferInit(&CompN,CompNArray,DEM_FSK_V1_TAP_COMP_N,DEM_FSK_V1_COMP_SIZE,false);//lo inicializo con 1
    gpioMode(PIN_LED_RED,OUTPUT);//todo:sacar
    gpioMode(PIN_SCK,OUTPUT);
    adcInit(ADC_0, 0,demFskV1AdcCallback,true,1);//inicializo el adc que voy a usar




}

static float aplyFilter(int32_t * dato){

    float temp=0;
    for (int i = 0; i < mySize; ++i) {
        temp += dato[i] * Bfilter[i];
    }
    return  temp;
}


//funcion que se llama del main, se ejecuta si solo e adc hizo una conversion
void demFskV1Run(void){
	if(hayMedicion){

		adcMeasData=adcGetData(ADC_0);
		gpioWrite(PIN_SCK, true);
		demFskV1AddSample(adcMeasData-DEM_FSK_V1_SIGNAL_OFSET); //agrego la muestra para que sea procesada, le resto el ofset
		gpioWrite(PIN_SCK, false);
		//todo:faltaria algo
		hayMedicion=false;
	}
}

void demFskV1AdcCallback(void){//el adc me avisa que llego una medicion
	hayMedicion=true;
}

static void interpData(int32_t sample){
	 static states estadoActual=DEM_IDLE;
	//sincronizacion con el  bitStream
	 switch(estadoActual){
	 case DEM_IDLE:
		 break;
	 case DEM_GET_BIT:
		 break;
	 case DEM_PAR:
		 break;
	 }
}

