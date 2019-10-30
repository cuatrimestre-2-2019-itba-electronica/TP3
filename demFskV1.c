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
#define DEM_FSK_V1_TAP_COMP_N 9//las nueve anteriores muestras
#define DEM_FSK_V1_COMP_SIZE 100

#define DEM_FSK_DATOS_RECIVIDOS_SIZE 100

#define DEM_FSK_SAMPLE_IGNORE 100

typedef enum{DEM_IDLE,DEM_GET_BIT,DEM_PAR}states;



demCircBufferBuff xN; //senala generada por el adc
int32_t xNArray[DEM_FSK_V1_XSIZE];

demCircBufferBuff yN;//senal xn * xn-5
int32_t yNArray[DEM_FSK_V1_YSIZE];

demCircBufferBuff CompN;//senal despues del comparador
int32_t CompNArray[DEM_FSK_V1_COMP_SIZE];

uint16_t adcMeasData=0; //almaceno la muestra que midio el adc
bool hayMedicion=false; //si es true indica que hay muestra para procesar


demCircBufferBuff datosN;//datos recividos
int32_t datosNArray[DEM_FSK_DATOS_RECIVIDOS_SIZE];

bool startConversion=false;
uint32_t samples2Ignore=0;

void demFskV1AdcCallback(void);
static float aplyFilter(int32_t * dato);
static void interpData(void);
static bool haystart(int32_t * buff);
static bool getBits(demCircBufferBuff * cirBuffSt, uint8_t * dRecived);
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

        if(!(aplyFilter(tempBuf)>0)){//salida del comparador
        	gpioWrite(PIN_SCK, true);
        	if(startConversion){
        		demCircBufferApend(&CompN,1);
        	}

        }else{
        	gpioWrite(PIN_SCK, false);
        	if(startConversion){
        		demCircBufferApend(&CompN,0);
        	}

        }

    }



}

void demFskV1Init(void) {
    demCircBufferInit(&xN,xNArray,DEM_FSK_V1_XSIZE,DEM_FSK_V1_TAP_XN,true);
    demCircBufferInit(&yN,yNArray,DEM_FSK_V1_YSIZE,DEM_FSK_V1_TAP_YN,true);
    demCircBufferInit(&CompN,CompNArray,DEM_FSK_V1_COMP_SIZE,DEM_FSK_V1_TAP_COMP_N,false);//lo inicializo con 1
    demCircBufferInit(&datosN,datosNArray,DEM_FSK_DATOS_RECIVIDOS_SIZE,3,true);//lo inicializo con 1
    //gpioMode(PIN_LED_RED,OUTPUT);//todo:sacar
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
		if(samples2Ignore>DEM_FSK_SAMPLE_IGNORE-1){
			startConversion=true;
		}
		adcMeasData=adcGetData(ADC_0);
		//gpioWrite(PIN_SCK, true);
		demFskV1AddSample(adcMeasData-DEM_FSK_V1_SIGNAL_OFSET); //agrego la muestra para que sea procesada, le resto el ofset
		if(startConversion){
			interpData();
		}else{
			samples2Ignore++;
		}


	 //gpioWrite(PIN_SCK, false);
		//todo:faltaria algo
		hayMedicion=false;
	}
}

void demFskV1AdcCallback(void){//el adc me avisa que llego una medicion
	hayMedicion=true;
}

static void interpData(void){
	 static states estadoActual=DEM_IDLE;
	 static uint8_t dr=0;
	 int i=2;
	 int32_t tempBuffer[13]; //buffer temporal donde almaceno la informacion para analizar
	 switch(estadoActual){
	 case DEM_IDLE:
		 demCircBufferGetDataBetweenPtr(&CompN,tempBuffer);//levanto de a 10 datos del buffer
		 if(haystart(tempBuffer)){//si huvo start paso a encontrar los bits
			 estadoActual=DEM_GET_BIT;
		 }
		 break;
	 case DEM_GET_BIT:
		 if(getBits(&CompN, &dr)){
			 estadoActual=DEM_IDLE;//todo:cambiar los estados
			 demCircBufferApend(&datosN,dr);
		 }
		// estadoActual=DEM_IDLE;
		 break;
	 case DEM_PAR:
		 break;
	 }
}

static bool haystart(int32_t * buff){
	int32_t acum=0;
	for(int i=0;i<DEM_FSK_V1_TAP_COMP_N+1;i++){//sumo todas las muestras
		acum+=buff[i];
	}
	if (acum>1){
		return false;
	}else{
		return true; //si hay mas ceros que unos entonces asumo cero y es start
	}
}

static bool getBits(demCircBufferBuff * cirBuffSt, uint8_t * dRecived){
	static uint8_t sampleCounter=0;
	static uint8_t bitCounter=0;
	int32_t tempSample=0;
	int32_t tempTSample=0;
	int32_t tempBufferr[13];
	uint8_t acum=0; //acumulador para disernir entre cero y uno
	uint8_t bitRecived=0;
	static uint8_t byteRecived=0;//variable donde guaro el byte acumulado

	demCircBufferGetData(cirBuffSt,&tempSample,&tempTSample);//vacio el buffer cada vez que me llaman

	if(sampleCounter==9){//si hay 10 muestras es 1 tiempo de bit (833us)
		demCircBufferGetDataBetweenPtrSinInc(cirBuffSt,tempBufferr);//agarro las 10 muestras, pero sin incremento
		acum=tempBufferr[3]+tempBufferr[4]+tempBufferr[5];//+tempBufferr[6]+tempBufferr[7]; //promedio las muestras centrales
		//for(int i=0;i<8;i++){
			//acum+=tempBufferr[i];
		//}
		if(acum>1){//uart
			bitRecived=1;
		}else{ //recivi un uno
			bitRecived=0;
		}



		byteRecived=byteRecived|bitRecived<<bitCounter;// armo el byte
		if(bitCounter==7){//recibi todos los bits
			bitCounter=0;//reinicio contadores
			sampleCounter=0;
			(*dRecived)=byteRecived;
			byteRecived=0;
			return true;
		}else{
			bitCounter++;
		}


		sampleCounter=0;
	}else{
		sampleCounter++;
	}
	return false;

}
